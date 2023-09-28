// #include "service/fwservice.hpp"
// #include "service/configservice.hpp"
// #include "service/rqueue.hpp"
// #include "agentUtils.hpp"
// #include "service/commandservice.hpp"
// #include "controller/logController.hpp"
// #include "controller/monitorController.hpp"
// #include "service/loganalysis.hpp"
// #include "controller/monitorController.hpp"

// // int main()
// // {
// //     Command command;
// //     vector<string> logs;
// //     command.readCommand("aa-status",logs);
// //     for(string line: logs){ cout << line << "\n";}
// // }
// int main()
// {
//     AgentUtils::syslog_enabled = true;
//     AgentUtils::setupLogger();
//     if (!AgentUtils::syslog_enabled)
//     {
//         AgentUtils::logfp.open(LOG_PATH, std::ios::app);
//     }
//     Timer timer;
//     Config config;
//     // MonitorController controller;
//     // map<string, map<string, string>> table;
//     // config.readIniConfigFile("/home/krishna/security/Agent/config/rules.config", table);
//     // controller.getMonitorLog(table);
//     LogAnalysis logAnalysis;
//     std::unordered_map<string, std::unordered_map<int, AConfig>> table;
//     std::unordered_map<string, decoder> dTable;
//     logAnalysis.start("/home/krishna/security/Agent/decoder.xml", "/home/krishna/security/Agent/rules/syslog_rules.xml", "/home/krishna/security/Agent/config/dpkg.log");
//     // Queue queue(
//     //     "/home/krishna/Downloads/rabbit-keys/keys/ca/ca_cert.pem",
//     //     "/home/krishna/Downloads/rabbit-keys/keys/server/server_cert.pem",
//     //     "/home/krishna/Downloads/rabbit-keys/keys/server/private/server_key.pem");
//     // queue.send("/home/krishna/security/Agent/process.json", "monitorQueue");
    
//     if (AgentUtils::logfp.is_open())
//     {
//         AgentUtils::logfp.close();
//     }
//     return 0;
// }