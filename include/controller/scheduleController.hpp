#ifndef SCHEDULAR_HPP
#define SCHEDULAR_HPP

#pragma once

#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "controller/firmwareController.hpp"
#include "service/configservice.hpp"

/**
 * @brief Schedule Manager
 *
 * The `Schedule` class is responsible for managing activities defined in a scheduler configuration file. It executes
 * these activities based on the time patterns defined in the configuration. This class serves as the core component
 * for scheduling and automating tasks or activities in your application.
 */
class Schedule
{

private:
    LogController _logController;         /**< A private instance of the LogController class. */
    MonitorController _monitorController; /**< A private instance of the MonitorController class. */
    FirmwareController _fController;      /**< A private instance of the FirmWareController class. */
    Config _configService;                /**< A private instance of the IniConfig class. */
    map<string, map<string, string>> _configTable;
    bool _isReadyToSchedule = true; /**< A private variable for configuration file status*/

private:
    /**
     * @brief Run Scheduled Process
     *
     * The `run` function maps a process to its corresponding controller based on `processName` and executes the task
     * defined by the process at the specified `timePattern`. If any process returns a failure result, the `index` is used
     * to indicate the corresponding thread index in the `processStatus` vector.
     *
     * @param[in] processName The name of the process to be executed.
     * @param[in] timePattern The time pattern defining when the process should be executed.
     * @param[in] index The thread index associated with the process in the `processStatus` vector if the process returns a failure result.
     * @param[out] processStatus A vector containing thread status information.
     */
    void run(const string &processName, const string &timePattern, int index, vector<bool> &processStatus);
    void printTime(std::chrono::system_clock::time_point &t);

    /**
     * @brief Process Time Pattern
     *
     * The `processTimePattern` function converts a string-formatted time pattern into a usable `patternTable` that contains
     * values for seconds, minutes, and hours. This conversion allows the application to work with and interpret time patterns
     * specified in a human-readable format.
     *
     * @param[out] patternTable A vector to store the time pattern values (seconds, minutes, hours).
     * @param[in] pattern The string-formatted time pattern to be processed and converted.
     * @return An integer result code:
     *         - SUCCESS: The time pattern was successfully processed and converted.
     *         - FAILED: The processing encountered errors or an invalid time pattern format.
     */
    int processTimePattern(vector<int> &patternTable, const string &pattern);
    // void task(const string &processName, std::vector<bool> &processStatus, int index, LogController &_logController, MonitorController &_monitorController, FirmWareController &_fController, IniConfig &_configService, map<string, map<string, string>> &_configTable);

public:
    Schedule() = default;
    /**
     * @brief Schedule Manager Constructor
     *
     * The `Schedule` class constructor initializes the schedule manager using a configuration file. This file contains
     * information about scheduled activities and their associated time patterns.
     *
     * @param[in] file The path to the configuration file that defines scheduled activities and time patterns.
     */
    Schedule(const string &file);

    /**
     * @brief Start Scheduled Activities
     *
     * The `start` function creates threads and maps them to their respective controllers based on their names. The number
     * of threads is determined by the sections specified in the scheduler configuration file, which is in INI format.
     * Each section corresponds to a controller responsible for specific activities. Controller mapping is achieved by
     * invoking the `run` function for each controller name found in the configuration.
     *
     * This function serves as the entry point for initializing and orchestrating the various activities of the agent
     * application in parallel.
     */
    void start();
    /**
     * @brief Destructor for Schedule.
     *
     * The destructor performs cleanup tasks for the `Schedule` class.
     */
    virtual ~Schedule();
};

Schedule::Schedule(const string &file)
{
    if (_configService.readConfigFile(file, _configTable) != SUCCESS)
    {
        _isReadyToSchedule = false;
    }
}

void Schedule::run(const string &processName, const string &timePattern, int index, vector<bool> &processStatus)
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
                        std::cout << "Monitor Log collected successfully." << "\n";
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading Process details operation stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(), "applog") == 0)
                {
                    if (_logController.appLogManager(_configTable) == SUCCESS)
                    {
                        std::cout << "Applog operation done" << "\n";
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading AppLog process stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(), "syslog") == 0)
                {
                    if (_logController.getSysLog(_configTable) == SUCCESS)
                    {
                        std::cout << "Syslog operation done" << "\n";
                    }
                    else
                    {
                        AgentUtils::writeLog("Reading SysLog process stopped");
                        processStatus[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(processName.c_str(), "firmware") == 0)
                {
                    int response = _fController.start(_configTable);
                    if (response == SUCCESS)
                    {
                        std::cout << "FirmWare operation done" << "\n";
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
                        break;                        // Exit the loop immediately
                    }
                }
            }

            std::cout << processName << " execution done." << "\n";

        } // end of while loop
        // Additional check after the loop to write the log message if the process failed
        if (!processStatus[index])
        {
            AgentUtils::writeLog(processName + " execution stopped from being runnig", FAILED);
        }
    }
    catch (const std::exception &e)
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
    cout << buffer << "\n";
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
            //            std::cout << processName << " : " << processTimePattern << "\n";
            threads[i] = std::thread([&, processName, processTimePattern]()
                                     {
                if (processStatus[i]) 
                {
                    run(processName, processTimePattern, i, processStatus); 
                } });
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << "\n";
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

    std::cout << "done" << "\n";
}

Schedule::~Schedule() {}

#endif