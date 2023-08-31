#ifndef LINMONITOR_HPP
#define LINMONITOR_HPP
#pragma once

#include "agentUtils.hpp"

#define MAX_NICE_VALUE 20
#define CLK_TCK 100
#define UITME 13
#define STIME 14
#define CUTIME 15
#define CSTIME 16
#define NICETIME 18
#define START_TIME 21

typedef struct process_data process_data;
typedef struct sys_properties sys_properties;

struct sys_properties
{
    double ram;
    double disk;
    double cpu;
};

struct process_data
{
    string processId;
    string processName;
    string cpuTime;
    string memUsage;
    string diskUsage;

    process_data(string id, string name, string cTime, string mUsage, string dUsage) 
                : processId(id), processName(name), cpuTime(cTime), memUsage(mUsage), diskUsage(dUsage)
    {}

};

class CpuTable
{
private:
    int _utime;
    int _stime;
    int _cutime;
    int _cstime;
    int _startTime;
    int _niceTime;
    int _upTime;
    int _cpuCount;
    int _getUpTime();

public:
    CpuTable(int utime, int stime, int cutime, int cstime, int niceTime, int startTime)
        : _utime(utime), _stime(stime), _cutime(cutime), _cstime(cstime), _startTime(startTime)
    {
        this->_upTime = _getUpTime();
        this->_cpuCount = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }

    CpuTable() {}

    int getUTime() { return _utime; }
    int getSTime() { return _stime; }
    int getCuTime() { return _cutime; }
    int getCsTime() { return _cstime; }
    int getStartTime() { return _startTime; }
    int getUpTime() { return _upTime; }
    int getCpuCount() { return _cpuCount; }
    int getNiceTime() { return _niceTime; }
};

class IMonitor
{
public:
    virtual int getData(const string& writePath, const vector<string>& columns) = 0;
    virtual sys_properties getSystemProperties() = 0;
    virtual sys_properties getAvailedSystemProperties() = 0;

    virtual ~IMonitor() {}
};

class MonitorService : public IMonitor
{
private:
    vector<std::future<void>> asyncTasks;
    CpuTable _table;
    double _cpuTime;
    int _saveLog(const vector<process_data>& logs, const vector<string>& columns);
    string _getProcesNameById(const unsigned int& processId);
    vector<int> _getProcessIds();
    CpuTable _readProcessingTimeById(const unsigned int& processId);
    double _calculateCpuTime(CpuTable& table);
    double _getMemoryUsage(const unsigned int& processId);
    double _getDiskUsage(const unsigned int& processId);
    string _getFileName();

public:
    MonitorService() {}
    void create_process_data(int processId, vector<process_data>& data);
    int getData(const string& writePath, const vector<string>& columns);
    sys_properties getSystemProperties();
    sys_properties getAvailedSystemProperties();
    virtual ~MonitorService();
};

#endif