#ifndef LOGSERVICE_HPP
#define LOGSERVICE_HPP
#pragma once

#include "agentUtils.hpp"
#include "service/configservice.hpp"
#include "udp.hpp"

#define MAX_RECORD_LIMIT 100

class ILog
{
public:
    virtual int getSysLog(
        string appName,
        Json::Value &json,
        vector<string> names,
        const string path,
        string &previousTime,
        vector<string> levels,
        const char remote) = 0;
    virtual int getAppLog(
        Json::Value &json,
        vector<string> names,
        const string readDir,
        const string writePath,
        string &previousTime,
        vector<string> levels,
        const char delimeter) = 0;
    virtual ~ILog() {}
};

class LogService : public ILog
{
private:
    IniConfig _configService;
    const char *STANDARD_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    map<string, int> _logLevel{{"none", 0}, {"trace", 1}, {"debug", 2}, {"warning", 3}, {"error", 4}, {"critical", 5}};
    map<string, int> _priorityLevel{{"none", 0}, {"trace", 1}, {"interaction", 2}, {"standard", 3}, {"alarm", 4}};
    map<string, int> _logCategory{{"sys", 2}, {"network", 3}, {"ufw", 4}};

private:
    int _saveLog(Json::Value &json, const string path, const vector<string> logs, const vector<string> columns, char delimeter);
    int _transportLog(const string jsonFile, const string appName);
    std::time_t _convertToTime(const std::string &datetime);
    int _readSysLog(Json::Value &json, string path, vector<string> &logs, const char delimeter, string &previousTime, bool &flag, vector<string> levels, string &nextReadingTime);
    int _readAppLog(Json::Value &json, string path, vector<string> &logs, const char delimeter, string previousTime, bool &flag, vector<string> levels, string &nextReadingTime);
    vector<std::filesystem::directory_entry> _getDirFiles(const string directory);
    int saveToLocal(vector<string> logs, const string appName);
    // int handleLocalLogFile(int day, int month, int year, string& filePath);
    int verifyJsonPath(string &timestamp);
    bool isPriorityLog(string &line, vector<string> levels, Json::Value &json);

    // int createLogFile(int day, int month, int year, string &filePath);

public:
    LogService() = default;
    // int getSysLog(const string appName, Json::Value &json, vector<string> names, const string path, string &previousTime, vector<string> levels, const char delimeter);
    int getSysLog(string appName, Json::Value &json, vector<string> names, const string path, string &previousTime, vector<string> levels, const char remote);
    int getAppLog(Json::Value &json, vector<string> names, const string readDir, const string writePath, string &previousTime, vector<string> levels, const char delimeter);
    int readDpkgLog(const string path, vector<string> &logs, string &previousTime, string &nextReadingTime, bool &flag);
    int readRemoteSysLog(UdpQueue &queue, vector<string> &logs);
    ~LogService();
};

#endif