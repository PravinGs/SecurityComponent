#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
int main()
{
    Timer timer;
    MonitorController controller;
    IniConfig config;
    map<string, map<string, string>> table;
    OS::CurrentDay = 31;
    OS::CurrentMonth = 8;
    OS::CurrentYear = 2023;
    config.readConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    controller.getMonitorLog(table);
    return 0;
}