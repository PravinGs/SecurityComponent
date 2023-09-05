#ifndef PROCESSES_CHECK
#define PROCESSES_CHECK

#pragma once

#include "utils/agentUtils.hpp"

class ProcessCheck
{
    private:
        int total;
        int error;
        pid_t max_pid = MAX_PID;
        int noproc = 1;
        int proc_pid_found;
        string proc_0 = "/proc";
        string proc_1 = "/proc/1";
        
    public:

        bool isDigit(string val)
        {
            bool result = false;
            try
            {
                std::stoi(val);
                result = true;
            }catch(exception &e)
            {
                cerr<< e.what() << endl;
            }
            return result;
        }
        
        int proc_read(int pid)
        {
            if (noproc) return SUCCESS;

            string directory = "/proc";

            if (std::filesystem::exists(directory) && std::filesystem::exists(directory + "/" + std::to_string(pid))) return SUCCESS;

            return FAILED;

        }

        int proc_stat(int pid)
        {
            if (noproc) return SUCCESS;

            string filePath = "/proc/" + std::to_string(pid);

            return OS::isDirExist(filePath);
        }

        int proc_chdir(int pid)
        {
            int ret = 0;
            char curr_dir[OS_SIZE_1024 + 1];
            char dir[OS_SIZE_1024 + 1];

            if (noproc) {
                return (0);
            }
            if (getcwd(curr_dir, OS_SIZE_1024) == NULL) {
                return (0);
            }
            if (chdir("/proc") == -1) {
                return (0);
            }

            snprintf(dir, OS_SIZE_1024, "/proc/%d", pid);
            if (chdir(dir) == 0) {
                ret = 1;
            }

            /* Returning to the previous directory */
            if (chdir(curr_dir) == -1) {
                return (0);
            }

            return (ret);
        }

        int read_proc_file(string file_name, string pid, int position)
        {
            struct stat statbuf;
            if (lstat(file_name.c_str(), &statbuf) < 0) {
                return (-1);
            }

            /* If directory, read the directory */
            if (S_ISDIR(statbuf.st_mode)) {
                return (read_proc_dir(file_name, pid, position));
            }
            return (0);
        }

        int read_proc_dir(string dir_name, string pid, int position)
        {
            int result = SUCCESS;
            if (dir_name.empty() || dir_name.length() > PATH_MAX) 
            {
                string error = dir_name;
                AgentUtils::writeLog(INVALID_PATH + error, FAILED);
                return -1;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir_name)) 
            {
                const auto& entry_name = entry.path().filename().string();

                // Ignore . and ..
                if (strcmp(entry_name.c_str(), ".") == 0 || strcmp(entry_name.c_str(),"..")) {
                    continue;
                }

                if (position == PROC_)
                {   
                    cout << entry_name << endl;
                    if (!isDigit(entry_name)) continue;
                    string fileName = dir_name +  "/" + entry_name;
                    result = read_proc_file(fileName, pid, position + 1);
                }else if (position == PID) {
                    if (strcmp(entry_name.c_str(), "task") == 0) {
                        string fileName = dir_name + "/" + entry_name;
                        read_proc_file(fileName, pid, position + 1);
                    }
                } else if (position == TASK) {
                    /* Check under proc/pid/task/lwp */
                    if (strcmp(entry_name.c_str(), pid.c_str()) == 0) {
                        proc_pid_found = 1;
                        break;
                    }
                } else {
                    break;
                }
            }
            return result;
        }

        int check_rc_readproc(int pid)
        {
            char char_pid[32];

            proc_pid_found = 0;

            /* NL threads */
            snprintf(char_pid, 31, "/proc/.%d", pid);
            fstream file(char_pid, std::ios::in);
            if (!file) return 1;
            else { file.close(); } 

            snprintf(char_pid, 31, "%d", pid);
            read_proc_dir("/proc", char_pid, PROC_);
            return (proc_pid_found);
        }

        int loopAllPids(string ps)
        {
            int _kill0 = 0; 
            int _kill1 = 0;
            int _gsid0 = 0;
            int _gsid1 = 0;
            int _gpid0 = 0;
            int _gpid1 = 0;
            int _ps0 = -1;
            int _proc_stat  = 0;
            int _proc_read  = 0;
            int _proc_chdir = 0;

            pid_t i = 1;
            pid_t my_pid;

            char command[OS_SIZE_1024 + 1];

            my_pid = getpid();

            for (;; i++) 
            {
                cout << i << endl;
                if ((i <= 0) || (i > max_pid)) {
                    break;
                }

                total++;

                _kill0 = 0;
                _kill1 = 0;
                _gsid0 = 0;
                _gsid1 = 0;
                _gpid0 = 0;
                _gpid1 = 0;
                _ps0 = -1;

                /* kill test */
                if (!((kill(i, 0) == -1) && (errno == ESRCH))) {
                    _kill0 = 1;
                }

                /* getsid test */
                if (!((getsid(i) == -1) && (errno == ESRCH))) {
                    _gsid0 = 1;
                }

                /* getpgid test */
                if (!((getpgid(i) == -1) && (errno == ESRCH))) {
                    _gpid0 = 1;
                }

                /* /proc test */
                _proc_stat = proc_stat(i);
                _proc_read = proc_read(i);
                _proc_chdir = proc_chdir(i);

                /* If PID does not exist, move on */
                if (!_kill0     && !_gsid0     && !_gpid0 &&
                        !_proc_stat && !_proc_read && !_proc_chdir) {
                    continue;
                }

                /* Ignore our own pid */
                if (i == my_pid) {
                    continue;
                }

                /* Check the number of errors */
                if (error > 15) {
                    string error_msg = "Excessive number of hidden processes. It maybe a false-positive or something really bad is going on.";
                    AgentUtils::writeLog(error_msg, CRITICAL);
                    return FAILED;
                }

                /* Check if the process appears in ps(1) output */
                if (!ps.empty()) {
                    snprintf(command, OS_SIZE_1024, "%s -p %d > /dev/null 2>&1", ps.c_str(), (int)i);
                    _ps0 = 0;
                    if (system(command) == 0) {
                        _ps0 = 1;
                    }
                }

                /* Everything fine, move on */
                if (_ps0 && _kill0 && _gsid0 && _gpid0 && _proc_stat && _proc_read) {
                    continue;
                }

                /*
                * If our kill or getsid system call got the PID but ps(1) did not,
                * find out if the PID is deleted (not used anymore)
                */
                if (!((getsid(i) == -1) && (errno == ESRCH))) {
                    _gsid1 = 1;
                }
                if (!((kill(i, 0) == -1) && (errno == ESRCH))) {
                    _kill1 = 1;
                }
                if (!((getpgid(i) == -1) && (errno == ESRCH))) {
                    _gpid1 = 1;
                }

                _proc_stat = proc_stat(i);
                _proc_read = proc_read(i);
                _proc_chdir = proc_chdir(i);

                /* If it matches, process was terminated in the meantime, so move on */
                if (!_gsid1 && !_kill1 && !_gpid1 && !_proc_stat &&
                        !_proc_read && !_proc_chdir) {
                    continue;
                }

                if (_gsid0 == _gsid1 &&
                        _kill0 == _kill1 &&
                        _gsid0 != _kill0) {
                    /* If kill worked, but getsid and getpgid did not, it may
                    * be a defunct process -- ignore.
                    */
                    if (! (_kill0 == 1 && _gsid0 == 0 && _gpid0 == 0) ) {
                        char op_msg[OS_SIZE_1024 + 1];

                        snprintf(op_msg, OS_SIZE_1024, "Process '%d' hidden from "
                                "kill (%d) or getsid (%d). Possible kernel-level"
                                " rootkit.", (int)i, _kill0, _gsid0);
                        AgentUtils::writeLog(op_msg, CRITICAL);
                        error++;
                    }
                } else if (_kill1 != _gsid1 ||
                        _gpid1 != _kill1 ||
                        _gpid1 != _gsid1) {
                    /* See defunct process comment above */
                    if (! (_kill1 == 1 && _gsid0 == 0 && _gpid0 == 0 && _gsid1 == 0) ) {
                        char op_msg[OS_SIZE_1024 + 1];

                        snprintf(op_msg, OS_SIZE_1024, "Process '%d' hidden from "
                                "kill (%d), getsid (%d) or getpgid. Possible "
                                "kernel-level rootkit.", (int)i, _kill1, _gsid1);
                        AgentUtils::writeLog(op_msg, CRITICAL);
                        error++;
                    }
                } else if (_proc_read != _proc_stat  ||
                        _proc_read != _proc_chdir ||
                        _proc_stat != _kill1) {
                    /* Check if the pid is a thread (not showing in /proc */
                    if (!noproc && !check_rc_readproc((int)i)) {
                        char op_msg[OS_SIZE_1024 + 1];

                        snprintf(op_msg, OS_SIZE_1024, "Process '%d' hidden from "
                                "/proc. Possible kernel level rootkit.", (int)i);
                        AgentUtils::writeLog(op_msg, CRITICAL);
                        error++;
                    }
                } else if (_gsid1 && _kill1 && !_ps0) {
                    /* checking if the pid is a thread (not showing on ps */
                    if (!check_rc_readproc((int)i)) {
                        char op_msg[OS_SIZE_1024 + 1];

                        snprintf(op_msg, OS_SIZE_1024, "Process '%d' hidden from "
                                "ps. Possible trojaned version installed.",
                                (int)i);
                        AgentUtils::writeLog(op_msg, CRITICAL);
                        error++;
                    }
                }
            }

            return SUCCESS;

        }
    
        int check()
        {
            int result = SUCCESS;
            string location = "/bin/ps";

            if (!std::filesystem::exists(location))
            {
                string temp = "/usr" + location;
                location = temp;
                if (!std::filesystem::exists(location))
                {
                    location = "";
                }
            }

            if (std::filesystem::exists(proc_0) && std::filesystem::exists(proc_1)) { noproc = 0; }

            if ((result = loopAllPids(location)))
            {
                AgentUtils::writeLog("Successfully process check completed.", INFO);
            }
            if (error == 0) 
            {
                string op_msg = "No hidden process by Kernel-level rootkits." + location + " is not trojaned. Analyzed " + std::to_string(total) + " processes.";
                AgentUtils::writeLog(op_msg, SUCCESS);
            }
            return result;
        }
};

#endif