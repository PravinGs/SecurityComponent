#ifndef LOGCONTROLLER_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "log-collector/logservice.hpp"
#include "utils/configservice.hpp"
#include "proxy/Proxy.hpp"
#include "connection/rqueue.hpp"

const string sysLogName = "syslog";

class LogController
{
private:
    ILog *_logService = nullptr;
    IniConfig _configService;
    Proxy _proxy;

public:
    LogController() : _logService(new LogService()) {}

    int sysLogManager(map<string, map<string, string>> configTable)
    {
        int result = SUCCESS;
        vector<string> syslogFiles = _configService.toVector(configTable["syslog"]["paths"], ',');
        vector<string> commands = _configService.toVector(configTable["syslog"]["commands"], ',');

        for (string file : syslogFiles)
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
            result = getSysLog(configTable, file, appName);
            std::chrono::seconds sleepDuration(2);
            std::this_thread::sleep_for(sleepDuration);
        }

        for (string command : commands)
        {
            cout << "Command : " << command << endl;
        }

        return result;
    }

    int getSysLog(map<string, map<string, string>> configTable, const string readPath, const string logName)
    {
        // long postResult;
        AgentUtils::writeLog("Reading " + logName + " starting...");
        Json::Value json;
        char remote;
        const string postUrl = configTable["cloud"]["log_url"];
        const string name = configTable["cloud"]["form_name"];
        string previousTime = _configService.trim(_proxy.getLastLogWrittenTime(logName, readPath));
        vector<string> names = _configService.toVector(configTable[sysLogName]["columns"], ',');
        vector<string> levels = _configService.toVector(configTable[sysLogName]["level"], ',');

        if (_proxy.isValidLogConfig(configTable, json, sysLogName, remote, previousTime) == FAILED)
            return FAILED;

        json["AppName"] = logName;

        if (_logService->getSysLog(logName, json, names, readPath, previousTime, levels, remote) == FAILED)
            return FAILED;

        // postResult = _proxy.post(postUrl, name, writePath); /*To read a json file path is required, so have previous time before update*/

        // if (postResult == POST_SUCCESS) { AgentUtils::updateLogWrittenTime(configTable[sysLogName]["last_time"], previousTime);}

        AgentUtils::updateLogWrittenTime(logName, previousTime);

        // _configService.cleanFile(writePath);

        // return postResult == POST_SUCCESS ? SUCCESS : FAILED;
        return SUCCESS;
    }

    int appLogManager(map<string, map<string, string>> table)
    {
        int result = SUCCESS;
        vector<string> apps = _configService.toVector(table["applog"]["list"], ',');

        for (string app : apps)
        {
            result = getAppLog(table, app);
        }
        return result;
    }

    int getAppLog(map<string, map<string, string>> configTable, string appLogName)
    {
        AgentUtils::writeLog("Reading " + appLogName + " log starting...");
        long postResult;
        Json::Value json;
        char sep = ' ';
        const string writePath = configTable[appLogName]["write_path"];
        const string readDir = configTable[appLogName]["log_directory"];
        const string postUrl = configTable["cloud"]["monitor_url"];
        const string attributeName = configTable["cloud"]["form_name"];
        vector<string> names = _configService.toVector(configTable[appLogName]["columns"], ',');
        vector<string> levels = _configService.toVector(configTable[appLogName]["level"], ',');
        string previousTime = _configService.trim(_proxy.getLastLogWrittenTime(configTable["cloud"]["name"], readDir));

        if (_proxy.isValidLogConfig(configTable, json, appLogName, sep, previousTime) == FAILED)
            return FAILED;

        if (_logService->getAppLog(json, names, readDir, writePath, previousTime, levels, sep) == FAILED)
            return FAILED;

        postResult = _proxy.post(postUrl, attributeName, writePath);

        if (postResult == POST_SUCCESS)
        {
            AgentUtils::updateLogWrittenTime(configTable[appLogName]["last_time"], previousTime);
        }

        _configService.cleanFile(writePath);

        return postResult == POST_SUCCESS ? SUCCESS : FAILED;
    }

    virtual ~LogController()
    {
        delete _logService;
    }
};

#endif
