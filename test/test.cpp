#include "common.hpp"
#include "service/config_service.hpp"
#include "controller/mqtt_controller.hpp"
#include "controller/log_controller.hpp"
#include "controller/monitor_controller.hpp"
#include "controller/mqtt_controller.hpp"
#include "controller/patch_controller.hpp"
#include "comm/server.hpp"
#include "comm/client.hpp"

void test_mqtt_controller()
{
    Config config;
    map<string, map<string, string>> table;
    config.read_ini_config_file("/etc/scl/config/agent/agent.config", table);
    mqtt_controller controller(table);
    controller.start();
}

void init()
{
    agent_utils::get_hostname(os::host_name);
    auto today = std::chrono::system_clock::now();
    auto timeInfo = std::chrono::system_clock::to_time_t(today);
    std::tm *tm_info = std::localtime(&timeInfo);
    int day = tm_info->tm_mday;
    os::current_day = day; /* Current day at the application starting date. */
    os::current_month = tm_info->tm_mon;
    os::current_year = tm_info->tm_year+1900;
    
}

void test_log_controller()
{
    log_controller controller("/home/pravin/micro-service/config/schedule.config");
    controller.start();
}

void test_process_controller()
{
    monitor_controller controller("/home/pravin/micro-service/config/schedule.config");
    controller.start();

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
    init();
    test_process_controller();
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.close();
    }
    closelog();
    return 0;
}