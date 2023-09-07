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
    LogAnalysis logAnalysis;
    std::unordered_map<string, std::unordered_map<int, AConfig>> table;
    logAnalysis.start("/home/krishna/security/Agent/decoder.xml", "/home/krishna/security/Agent/rules/", "/home/krishna/Documents/syslog");
    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    return 0;
}