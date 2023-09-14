#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "service/loganalysis.hpp"

int main()
{
    AgentUtils::syslog_enabled = true;
    AgentUtils::setupLogger();
    if (!AgentUtils::syslog_enabled)
    {
        AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    }
    Timer timer;
    Config config;
    LogAnalysis logAnalysis;
    std::unordered_map<string, std::unordered_map<int, AConfig>> table;
    std::unordered_map<string, decoder> dTable;
    logAnalysis.start("/home/pravin/Desktop/SecurityComponent/decoder.xml", "/home/pravin/Desktop/SecurityComponent/rules", "/home/pravin/Desktop/SecurityComponent/config/syslog");

    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    return 0;
}