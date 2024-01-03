#ifndef SYSFILES_CHECK
#define SYSFILES_CHECK
#pragma once


#include "common.hpp"

/*
    1.identify the files attendance.
    2.Source file neede to retrive the pre identified malware filepaths.
*/

class sys_check
{
    private:
        string rk_sys_file[MAX_RK_SYS +  1];
        string rk_sys_name[MAX_RK_SYS + 1];
        int rk_sys_count;
        const string base_dir = "/";
        int errors;
        int total;
    
    public:

        int check(const string source_file, vector<string> & reports)
        {
            total = 0;
            errors = 0;
            rk_sys_count = 0;
            fstream file(source_file, std::ios::in);

            if (!file)
            {
                agent_utils::write_log("sys_check: check: " + source_file + " not exists.", FAILED);
                return FAILED;
            }
            string line;
            while (std::getline(file, line))
            {
                total++;
                /* Check rk_sys_count for IOB exception */
                if (rk_sys_count >= MAX_RK_SYS)
                {
                    agent_utils::write_log("sys_check: check: rk_sys count reached", WARNING);
                    break;
                }
                /* continue comments and empty lines */
                if (line[0] == '#' || line.empty())
                {
                    continue;
                } 

                int mid = (int)line.find_first_of('!');
                string file = agent_utils::trim(line.substr(0, mid));
                int end = (int)line.find_first_of(':');
                string name = agent_utils::trim(line.substr(mid + 1, end - mid - 1));
                string filePath = base_dir + file;
                if (file[0] == '*')
                {
                    string temp = file.substr(file.find_first_of('/'));
                    rk_sys_file[rk_sys_count] = temp;
                    rk_sys_name[rk_sys_count] = name;
                    if (rk_sys_name[rk_sys_count].empty() || rk_sys_file[rk_sys_count].empty()) 
                    {
                        agent_utils::write_log("sys_check: check: could not acquire memory", WARNING);
                        rk_sys_file[rk_sys_count].clear();
                        rk_sys_name[rk_sys_count].clear();
                    }
                    rk_sys_count++;
                    continue;
                }

                if (std::filesystem::exists(filePath))
                {
                    reports.push_back(name+","+file);
                    agent_utils::write_log("sys_check: check: rootkit " + name + " detected by the presence of file " + file, CRITICAL);
                    errors++;
                }
            }

            file.close();

            agent_utils::write_log("sys_check: check: total " + std::to_string(total) + " number of rootkit files processed.");

            if (errors > 0)
            {
                agent_utils::write_log("sys_check: check: total " + std::to_string(errors) + " number of rootkit detected.");
            }
            return SUCCESS;
        }

};

#endif