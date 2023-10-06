// #include "controller/main_controller.hpp"
// #include "common.hpp"
// #ifndef NO_DEBUG
// #define NO_DEBUG
// #endif
// int main()
// {   
//     openlog("agent.service", LOG_INFO|LOG_CONS, LOG_USER);
//     AgentUtils::syslog_enabled = true;
//     AgentUtils::setupLogger();
//     if (!AgentUtils::syslog_enabled)
//     {
//         AgentUtils::logfp.open(LOG_PATH, std::ios::app);
//     }
    
//     MainController controller(AGENT_CONFIG_DIRECTORY);
//     controller.start();
//     if (AgentUtils::logfp.is_open())
//     {
//         AgentUtils::logfp.close();
//     }
//         closelog();
//     return 0;
// }