#ifndef SCHEDULAR_HPP
#define SCHEDULAR_HPP

#pragma once

#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "controller/firmwareController.hpp"
#include "service/configservice.hpp"

class Schedule
{
private:
    void run(std::string processName, std::string timePattern, int index, std::vector<bool> &processStatus);
    void printTime(std::chrono::system_clock::time_point &t);
    int processTimePattern(vector<int> &patternTable, const string &pattern);
    void task(const std::string &processName, std::vector<bool> &processStatus, int index, LogController &_logController, MonitorController &_monitorController, FirmWareController &_fController, IniConfig &_configService, map<string, map<string, string>> &_configTable);

public:
    LogController _logController;
    MonitorController _monitorController;
    FirmWareController _fController;
    IniConfig _configService;
    map<string, map<string, string>> _configTable;
    bool _isReadyToSchedule = true;
    Schedule() = default;
    Schedule(const string file);
    void start();
    virtual ~Schedule();
};

Schedule::Schedule(const string file)
{
    if (_configService.readConfigFile(file, _configTable) != SUCCESS)
    {
        _isReadyToSchedule = false;
    }
}

void Schedule::run(std::string processName, std::string timePattern, int index, std::vector<bool> &processStatus)
{
    try
    {
        auto cron = cron::make_cron(timePattern);
        while (processStatus[index])
        {
            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(currentTime);
            std::time_t next = cron::cron_next(cron, time);
            std::chrono::system_clock::time_point targetPoint = std::chrono::system_clock::from_time_t(next);
            std::chrono::duration<double> duration = targetPoint - currentTime;
            std::this_thread::sleep_for(duration);
            if (processStatus[index])
            {
                if (strcmp(processName.c_str(), "monitor") == 0)
                {
                    if (_monitorController.getMonitorLog(_configTable) == SUCCESS)
                    {
                        std::cout << "Monitor Log collected successfully." << std::endl;
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading Process details operation stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break; // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(), "applog") == 0)
                {
                    if (_logController.appLogManager(_configTable) == SUCCESS)
                    {
                        std::cout << "Applog operation done" << std::endl;
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading AppLog process stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break; // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(),"syslog") == 0)
                {
                    if (_logController.getSysLog(_configTable) == SUCCESS)
                    {
                        std::cout << "Syslog operation done" << std::endl;
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading SysLog process stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break; // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(), "firmware") == 0)
                {
                    int response = _fController.start(_configTable);
                    if (response == SUCCESS)
                    {
                        std::cout << "FirmWare operation done" << std::endl;
                    }
                    else if (response == SCHEDULAR_WAIT)
                    {
                        int timeoutMinutes = 30;
                        std::chrono::minutes timeoutDuration(timeoutMinutes);
                        std::this_thread::sleep_for(timeoutDuration);
                    }
                    else
                    {
                        /*Do something*/
                        processStatus[index] = false; // Mark the process as failed
                        break; // Exit the loop immediately
                    }
                }
            }

            std::cout << processName << " execution done." << std::endl;

        } // end of while loop
        // Additional check after the loop to write the log message if the process failed
        if (!processStatus[index])
        {
            AgentUtils::writeLog(processName + " execution stopped from being runnig", FAILED);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Schedule::printTime(std::chrono::system_clock::time_point &t)
{
    std::time_t currentTime = std::chrono::system_clock::to_time_t(t);
    struct std::tm *now_tm = std::localtime(&currentTime);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now_tm);
    cout << buffer << endl;
}

int Schedule::processTimePattern(vector<int> &patternTable, const string &pattern)
{
    int result = SUCCESS;
    if (pattern.length() == 0)
    {
        AgentUtils::writeLog("Invalid Pattern found [ " + pattern + " ]", FAILED);
        return FAILED;
    }
    int position = 0;
    string token;
    std::istringstream iss(pattern);
    while (std::getline(iss, token, ' '))
    {
        int processedToken;
        try
        {
            if (token.length() == 1)
            {
                if (token[0] == '*')
                    processedToken = 1;
                else
                {
                    processedToken = std::stoi(token);
                }
                patternTable.push_back(processedToken);
                position++;
                continue;
            }

            else
            {
                token = token.substr(2);
            }
            processedToken = std::stoi(token);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            string err = e.what();
            AgentUtils::writeLog("Invalid Pattern" + err, FAILED);
            result = FAILED;
            break;
        }
        patternTable.push_back(processedToken);
        position++;
    }
    return result;
}

void Schedule::start()
{
    if (!_isReadyToSchedule)
        return;

    std::vector<std::string> processes;
    std::map<std::string, std::string> schedular = _configTable["schedular"];
    for (const auto &process : schedular)
    {
        processes.push_back(process.first);
    }

    std::vector<std::thread> threads(processes.size());

    std::vector<bool> processStatus(processes.size(), true); // Track the status of each process

    for (int i = 0; i < (int)processes.size(); i++)
    {
        try
        {
            std::string processName = processes[i];
            std::string processTimePattern = schedular[processes[i]];
            //            std::cout << processName << " : " << processTimePattern << std::endl;
            threads[i] = std::thread([&, processName, processTimePattern]()
                                     {
                if (processStatus[i]) 
                {
                    run(processName, processTimePattern, i, processStatus); 
                } });
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            string error = e.what();
            AgentUtils::writeLog(error, FAILED);
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for tasks to start

    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    std::cout << "done" << std::endl;
}



Schedule::~Schedule() {}

#endif