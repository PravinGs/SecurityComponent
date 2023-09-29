#ifndef LOGCONTROLLER_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "service/logservice.hpp"
#include "service/configservice.hpp"
#include "proxy/Proxy.hpp"
#include "service/curlservice.hpp"

/**
 * @brief Log Controller
 *
 * The `LogController` class serves as the controller layer for managing and reading logs from system and application logs.
 * It provides methods for initiating log reading operations and managing log-related tasks.
 */
class LogController
{
private:
    ILog* _logService = nullptr; /**< A private pointer to the ILog service. */
    Config _configService; /**< A private instance of IniConfig for configuration management. */
    vector<std::future<int>> _asyncSysTasks; /**< A private vector of futures for asynchronous system tasks. */
    vector<std::future<int>> _asyncAppTasks; /**< A private vector of futures for asynchronous application tasks. */
    Proxy _proxy; /**< A private instance of the Proxy class. */
    const string sysLogName = "syslog"; /**< A private constant string for system log name. */

public:
    /**
     * @brief Construct a new LogController object.
     *
     * This constructor initializes the `LogController` and creates an instance of the `LogService`
     * to be used for log management.
     */
    LogController() : _logService(new LogService()) {}

    /**
     * @brief System Log Manager
     *
     * This function reads configured log files from the `configTable` and asynchronously invokes the `getSysLog` function
     * for each configured file. It manages the logging process and returns a result code.
     *
     * @param[in] configTable A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The logging process was successful for all configured files.
     *         - FAILED: The logging process encountered errors for one or more files.
     *
     * @see getSysLog
     */
    int sysLogManager(map<string, map<string, string>> &configTable)
    {
        int result = SUCCESS;
        vector<string> syslogFiles = _configService.toVector(configTable["syslog"]["paths"], ',');

        for (const string &file : syslogFiles)
        {
            string appName;
            if (file.size() == 0)
                continue;
            size_t start = file.find_last_of('/');
            size_t end = file.find_first_of('.');
            if (start != std::string::npos && end != std::string::npos)
            {
                appName = file.substr(start + 1, end - start - 1);
            }
            else if (start != std::string::npos && end == std::string::npos)
            {
                appName = file.substr(start + 1);
            }
            else
            {
                break;
            }
            auto asyncTask = [&, file, appName]() -> int
            {
                return getSysLog(configTable, file, appName);
            };
            _asyncSysTasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &asyncTask : _asyncSysTasks)
        {
            result = asyncTask.get();
        }

        return result;
    }

    /**
     * @brief Get System Log
     *
     * This function performs necessary validations for a log file specified by `logName` in the `configTable`. It reads
     * log data from the file located at `readPath` and processes it as needed.
     * 
     * After validating, it invokes the `getSysLog` function from the `_logService` instance to perform
     * the actual log collection. Finally, it sends the collected log data to the cloud for further processing.
     *
     * @param[in] configTable A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @param[in] readPath The path to the log file to be read.
     * @param[in] logName The identifier of the log file to be processed.
     * @return An integer result code:
     *         - SUCCESS: The log file was successfully validated, read, and processed.
     *         - FAILED: The validation or processing of the log file encountered errors.
     */
    int getSysLog(map<string, map<string, string>> &configTable, const string &readPath, const string &logName)
    {
        int result;
        AgentUtils::writeLog("Reading " + logName + " starting...", INFO);
        Json::Value json;
        char remote;
        const string postUrl = configTable["cloud"]["log_url"];
        const string name = configTable["cloud"]["form_name"];
        string previousTime = _configService.trim(_proxy.getLastLogWrittenTime(logName, readPath));
        vector<string> names = _configService.toVector(configTable[sysLogName]["columns"], ',');
        vector<string> levels = _configService.toVector(configTable[sysLogName]["level"], ',');
        result = _proxy.isValidLogConfig(configTable, json, sysLogName, remote, previousTime);
        if (result == FAILED)
            return result;

        json["AppName"] = logName;
        result = _logService->getSysLog(logName, json, names, readPath, previousTime, levels, remote);
        if (result == FAILED)
            return result;
        AgentUtils::updateLogWrittenTime(logName, previousTime);
        return result;
    }

    /**
     * @brief Application Log Manager
     *
     * This function reads configured log files from the `configTable` and asynchronously invokes the `getAppLog` function
     * for each configured file. It manages the logging process and returns a result code.
     *
     * @param[in] configTable A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The logging process was successful for all configured files.
     *         - FAILED: The logging process encountered errors for one or more files.
     *
     * @see getAppLog
     */
    int appLogManager(map<string, map<string, string>> &configTable)
    {
        int result = SUCCESS;
        vector<string> apps = _configService.toVector(configTable["applog"]["list"], ',');

        for (string app : apps)
        {
            
            auto asyncTask = [&, app]() -> int 
            {
                return getAppLog(configTable, app);
            };

            _asyncAppTasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &asyncTask : _asyncAppTasks)
        {
            result = asyncTask.get();
        }
        return result;
    }

    /**
     * @brief Get Application Log
     *
     * This function performs necessary validations for a log file specified by `logName` in the `configTable`. It reads
     * log data from the file located at `readPath` and processes it as needed.
     *
     * After validating, it invokes the `getSysLog` function from the `_logService` instance to perform
     * the actual log collection. Finally, it sends the collected log data to the cloud for further processing.
     * 
     * @param[in] configTable A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @param[in] logName The identifier of the log file to be processed.
     * @return An integer result code:
     *         - SUCCESS: The log file was successfully validated, read, and processed.
     *         - FAILED: The validation or processing of the log file encountered errors.
     */
    int getAppLog(map<string, map<string, string>> &configTable, const string &logName)
    {
        AgentUtils::writeLog("Reading " + logName + " log starting...", INFO);
        long postResult;
        Json::Value json;
        char sep = ' ';
        const string writePath = configTable[logName]["write_path"];
        const string readDir = configTable[logName]["log_directory"];
        const string postUrl = configTable["cloud"]["monitor_url"];
        const string attributeName = configTable["cloud"]["form_name"];
        vector<string> names = _configService.toVector(configTable[logName]["columns"], ',');
        vector<string> levels = _configService.toVector(configTable[logName]["level"], ',');
        string previousTime = _configService.trim(_proxy.getLastLogWrittenTime(configTable["cloud"]["name"], readDir));

        if (_proxy.isValidLogConfig(configTable, json, logName, sep, previousTime) == FAILED)
            return FAILED;

        if (_logService->getAppLog(json, names, readDir, writePath, previousTime, levels, sep) == FAILED)
            return FAILED;

        postResult = CurlHandler::post(postUrl, attributeName, writePath);

        if (postResult == POST_SUCCESS)
        {
            AgentUtils::updateLogWrittenTime(configTable[logName]["last_time"], previousTime);
        }

        _configService.cleanFile(writePath);

        return postResult == POST_SUCCESS ? SUCCESS : FAILED;
    }

    /**
     * @brief Destructor for LogController.
     *
     * The destructor performs cleanup tasks for the `LogController` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_logService` instance.
     */
    virtual ~LogController()
    {
        delete _logService;
    }
};

#endif
