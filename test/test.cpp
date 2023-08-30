#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
int main()
{
    Timer timer;
    LogController controller;
    IniConfig config;
    map<string, map<string, string>> table;
    OS::CurrentDay = 30;
    OS::CurrentMonth = 8;
    OS::CurrentYear = 2023;
    config.readConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    controller.sysLogManager(table);
    return 0;
}