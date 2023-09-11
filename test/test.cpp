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
    std::unordered_map<string, decoder> dTable;
    logAnalysis.setConfigFile("/home/pravin/Desktop/SecurityComponent/decoder.xml", "/home/pravin/Desktop/SecurityComponent/rules/");
    cout << dTable.size() << "\n";
    string log = "Sep  8 23:51:41 pravin-HP-Pavilion-Laptop-15-eh1xxx kernel: [ 3931.634114] [UFW BLOCK] IN=wlo1 OUT= MAC=01:00:5e:00:00:01:b4:a7:c6:df:74:96:08:00 SRC=192.168.29.1 DST=224.0.0.1 LEN=36 TOS=0x00 PREC=0xC0 TTL=1 ID=22836 PROTO=2 ";
    log_event l = logAnalysis.decodeLog(log, "syslog");

    // config.readXmlRuleConfig("/home/krishna/security/Agent/config/test-rules.xml", table);
    // logAnalysis.start
    //     ("/home/pravin/Desktop/SecurityComponent/decoder.xml",
    //      "/home/pravin/Desktop/SecurityComponent/rules/",
    //      "/home/pravin/Desktop/SecurityComponent/config/syslog");
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