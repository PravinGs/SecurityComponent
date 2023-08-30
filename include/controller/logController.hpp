#ifndef LOGCONTROLLER_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "service/logservice.hpp"
#include "service/configservice.hpp"
#include "proxy/Proxy.hpp"
#include "service/rqueue.hpp"

const string sysLogName = "syslog";

class LogController
{
private:
    ILog *_logService = nullptr;
    IniConfig _configService;
    vector<std::future<int>> asyncTasks;
    Proxy _proxy;

public:
    LogController() : _logService(new LogService()) {}

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
            asyncTasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &asyncTask : asyncTasks)
        {
            result = asyncTask.get();
        }

        return result;
    }

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

    int appLogManager(map<string, map<string, string>> &table)
    {
        int result = SUCCESS;
        vector<string> apps = _configService.toVector(table["applog"]["list"], ',');

        for (string app : apps)
        {
            result = getAppLog(table, app);
        }
        return result;
    }

    int getAppLog(map<string, map<string, string>> &configTable, const string &appLogName)
    {
        AgentUtils::writeLog("Reading " + appLogName + " log starting...", INFO);
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
