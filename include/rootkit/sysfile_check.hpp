#ifndef SYSFILES_CHECK
#define SYSFILES_CHECK
#pragma once


#include "common.hpp"

/*
    1.identify the files attendance.
    2.Source file neede to retrive the pre identified malware filepaths.
*/

class SysCheck
{
    private:
        string rk_sys_file[MAX_RK_SYS +  1];
        string rk_sys_name[MAX_RK_SYS + 1];
        int rk_sys_count;
        const string base_dir = "/";
        int errors;
        int total;
    
    public:

        int check(const string sourceFile, vector<string> & reports)
        {
            total = 0;
            errors = 0;
            rk_sys_count = 0;
            fstream file(sourceFile, std::ios::in);

            if (!file)
            {
                AgentUtils::writeLog(sourceFile + " not exists.", FAILED);
                return FAILED;
            }
            string line;
            while (std::getline(file, line))
            {
                total++;
                /* Check rk_sys_count for IOB exception */
                if (rk_sys_count >= MAX_RK_SYS)
                {
                    AgentUtils::writeLog("RK_SYS count reached", WARNING);
                    break;
                }
                /* continue comments and empty lines */
                if (line[0] == '#' || line.empty())
                {
                    continue;
                } 

                int mid = (int)line.find_first_of('!');
                string file = AgentUtils::trim(line.substr(0, mid));
                int end = (int)line.find_first_of(':');
                string name = AgentUtils::trim(line.substr(mid + 1, end - mid - 1));
                string filePath = base_dir + file;
                if (file[0] == '*')
                {
                    string temp = file.substr(file.find_first_of('/'));
                    rk_sys_file[rk_sys_count] = temp;
                    rk_sys_name[rk_sys_count] = name;
                    if (rk_sys_name[rk_sys_count].empty() || rk_sys_file[rk_sys_count].empty()) 
                    {
                        AgentUtils::writeLog("Could not acquire memory", WARNING);
                        rk_sys_file[rk_sys_count].clear();
                        rk_sys_name[rk_sys_count].clear();
                    }
                    rk_sys_count++;
                    continue;
                }

                if (std::filesystem::exists(filePath))
                {
                    reports.push_back(name+","+file);
                    AgentUtils::writeLog("Rootkit " + name + " detected by the presence of file " + file, CRITICAL);
                    errors++;
                }
            }

            file.close();

            AgentUtils::writeLog("Total " + std::to_string(total) + " number of rootkit files processed.");

            if (errors > 0)
            {
                AgentUtils::writeLog("Total " + std::to_string(errors) + " number of rootkit detected.");
            }
            return SUCCESS;
        }

};

#endif