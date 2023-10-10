// #ifndef NO_DEBUG
// #define NO_DEBUG
// #endif
// // // #include "service/fwservice.hpp"
// // // #include "service/configservice.hpp"
// // // #include "service/rqueue.hpp"
// #include "common.hpp"
// #include "service/root_check.hpp"
// // // #include "service/commandservice.hpp"
// // // #include "controller/logController.hpp"
// // // #include "controller/monitor_controller.hpp"
// // // #include "service/log_analysis.hpp"
// // // #include "controller/monitor_controller.hpp"

// // // // int main()
// // // // {
// // // //     Command command;
// // // //     vector<string> logs;
// // // //     command.read_command("aa-status",logs);
// // // //     for(string line: logs){ cout << line << "\n";}
// // // // }
// int main()
// {
//     openlog("agent", LOG_INFO|LOG_CONS, LOG_USER);
//     agent_utils::syslog_enabled = true;
//     agent_utils::setup_logger();
//     if (!agent_utils::syslog_enabled)
//     {
//         agent_utils::logfp.open(LOG_PATH, std::ios::app);
//     }
//     root_check check;
//     int result = check.start();
//     if (result == SUCCESS){
//         cout << "root_check success" << "\n";
//     }

//     if (agent_utils::logfp.is_open())
//     {
//         agent_utils::logfp.close();
//     }
//     closelog();
//     return 0;
// }