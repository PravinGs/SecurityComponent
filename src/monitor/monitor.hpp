#ifndef LINMONITOR_HPP
#define LINMONITOR_HPP
#pragma once

#include "utils/agentUtils.hpp"

#define MAX_NICE_VALUE 20
#define CLK_TCK 100
#define UITME 13
#define STIME 14
#define CUTIME 15
#define CSTIME 16
#define NICETIME 18
#define START_TIME 21

class IMonitor
{
public:
    virtual int getData(const string writePath, vector<string> columns) = 0;
    virtual Monitor::SYS_PROPERTIES getSystemProperties() = 0;
    virtual Monitor::SYS_PROPERTIES getAvailedSystemProperties() = 0;

    virtual ~IMonitor() {}
};

class MonitorService : public IMonitor
{
private:
    Monitor::CpuTable _table;
    double _cpuTime;
    vector<int> _processIds;
    int _saveLog(const string path, vector<Monitor::Data> logs, vector<string> columns);
    string _getProcesNameById(const unsigned int processId);
    void _getProcessIds();
    Monitor::CpuTable _readProcessingTimeById(const unsigned int processId);
    double _calculateCpuTime(Monitor::CpuTable table);
    double _getMemoryUsage(const unsigned int processId);
    double _getDiskUsage(const unsigned int processId);

public:
    MonitorService() {}
    int getData(const string writePath, vector<string> columns);
    Monitor::SYS_PROPERTIES getSystemProperties();
    Monitor::SYS_PROPERTIES getAvailedSystemProperties();
    virtual ~MonitorService();
};

#endif