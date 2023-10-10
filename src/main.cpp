#include "controller/main_controller.hpp"
#include "common.hpp"
#ifndef NO_DEBUG
#define NO_DEBUG
#endif
int main()
{   
    openlog("agent.service", LOG_INFO|LOG_CONS, LOG_USER);
    agent_utils::syslog_enabled = true;
    agent_utils::setup_logger();
    if (!agent_utils::syslog_enabled)
    {
        agent_utils::logfp.open(LOG_PATH, std::ios::app);
    }
    
    main_controller controller(AGENT_CONFIG_DIRECTORY);
    controller.start();
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.close();
    }
        closelog();
    return 0;
}