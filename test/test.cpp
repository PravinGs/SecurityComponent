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
    // config.readXmlRuleConfig("/home/krishna/security/Agent/config/test-rules.xml", table);
    logAnalysis.start
        ("/home/krishna/security/Agent/decoder.xml",
         "/home/krishna/security/Agent/config/test-rules.xml",
         "/home/krishna/Documents/dpkg.log");
    // string log = "2023-09-05 20:04:18 status unpacked graphviz:amd64 2.42.2-3build2";
    // string match;
    // size_t position = -1;
    // logAnalysis.pcreMatch(log, "^\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2} installed |", match, position);
    // cout << match << "\n";
    // if (position >= 0)
    // {
    //     cout << log.substr(position) << "\n";
    // }
    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    return 0;
}