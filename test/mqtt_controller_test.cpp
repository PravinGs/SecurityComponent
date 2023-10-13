#include "common.hpp"
#include "service/config_service.hpp"
#include "controller/mqtt_controller.hpp"
#include <gtest/gtest.h>

TEST( system_security_report, success )
{
    Config config;
    map<string, map<string, string>> table;
    config.read_ini_config_file("/etc/scl/config/agent/agent.config", table);
    mqtt_controller controller(table);
    controller.start();
}