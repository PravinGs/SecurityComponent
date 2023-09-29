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

//     MonitorController monitorController;
//     map<string, map<string, string>> table;
//     Config config;
//     config.readIniConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
//     monitorController.getMonitorLog(table);
//     if (AgentUtils::logfp.is_open())
//     {
//         AgentUtils::logfp.close();
//     }
//     return 0;
// }