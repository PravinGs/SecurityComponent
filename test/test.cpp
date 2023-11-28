#include "controller/agent.hpp"

// void test_mqtt_controller()
// {
//     Config config;
//     map<string, map<string, string>> table;
//     config.read_ini_config_file("/etc/scl/config/agent/agent.config", table);
//     mqtt_controller controller(table);
//     controller.start();
// }

void test_agent_interface()
{
    agent agent_if("/home/champ/SecurityComponent/config/schedule.config");
    
}

int main()
{
    openlog("agent.service", LOG_INFO | LOG_CONS, LOG_USER);
    agent_utils::syslog_enabled = true;
    agent_utils::setup_logger();
    if (!agent_utils::syslog_enabled)
    {
        agent_utils::logfp.open(LOG_PATH, std::ios::app);
    }
    test_agent_interface();
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.close();
    }
    closelog();
    return 0;
}