#include "common.hpp"
#include "service/config_service.hpp"
#include "controller/mqtt_controller.hpp"

void test_mqtt_controller()
{
    Config config;
    map<string, map<string, string>> table;
    config.read_ini_config_file("/etc/scl/config/agent/agent.config", table);
    mqtt_controller controller(table);
    controller.start();
}

// int main()
// {
//     test_mqtt_controller();
//     return 0;
// }