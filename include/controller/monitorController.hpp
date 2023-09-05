#ifndef MCONTROLLER_HPP
#define MCONTROLLER_HPP
#pragma once

#include "service/monitor.hpp"
#include "service/configservice.hpp"
#include "proxy/Proxy.hpp"

class MonitorController
{
private:
    IMonitor *_monitorService = nullptr;
    IniConfig _configService;
    Proxy proxy;
    const string monitor = "monitor";

public:
    MonitorController() : _monitorService(new MonitorService()) {}

    int getMonitorLog(map<string, map<string, string>> configTable)
    {
        int result = SUCCESS;
        string writePath = configTable[monitor]["write_path"];
        string postUrl = configTable["cloud"]["monitor_url"];
        string attributeName = configTable["cloud"]["form_name"];
        vector<string> columns = _configService.toVector(configTable[monitor]["columns"], ',');
        if (_monitorService->getData(writePath, columns) == FAILED)
            return FAILED;

        result = proxy.post(postUrl, attributeName, writePath);
        _configService.cleanFile(writePath);
        return result;
    }

    virtual ~MonitorController()
    {
        delete _monitorService;
    }
};

#endif