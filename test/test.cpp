#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "service/loganalysis.hpp"
int main()
{
    AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    Timer timer;
    Config config;
    map<string, map<string, string>> table;
    MonitorController logController;
    config.readIniConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    OS::CurrentDay = 6;
    OS::CurrentMonth = 9;
    logController.getMonitorLog(table);
    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    return 0;
}