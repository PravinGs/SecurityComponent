#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "service/loganalysis.hpp"
int main()
{
    Timer timer;
    Config config;
    map<string, map<string, string>> table;
    MonitorController monitorController;
    config.readIniConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    OS::CurrentDay = 4;
    OS::CurrentMonth = 9;
    monitorController.getMonitorLog(table);
    return 0;
}