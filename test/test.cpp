#include "common.hpp"
#include "service/config_service.hpp"
#include "controller/mqtt_controller.hpp"
#include "controller/log_controller.hpp"
#include "controller/monitor_controller.hpp"
#include "controller/mqtt_controller.hpp"
#include "controller/patch_controller.hpp"
#include "controller/tpm_controller.hpp"
#include "comm/server.hpp"
#include "comm/client.hpp"
#include "repository/signature_repository.hpp"

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
    os::current_year = tm_info->tm_year + 1900;
}

void test_log_controller()
{
    log_controller controller("/home/champ/SecurityComponent/config/schedule.config");
    controller.start();
}

void test_process_controller()
{
    monitor_controller controller("/home/pravin/micro-service/config/agent.config");
    controller.start();
}

void test_hmac_signature()
{
    string file = "/home/pravin/micro-service/config/schedule.config";
    string key = "pappadam";
    file_security security;
    security.sign_and_store_signature(file, key);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    if (!security.verify_signature(file, key))
    {
        agent_utils::write_log("test: test_hmac_signature: verify_signature: failed", WARNING);
    }
    else 
    {
        cerr << "verification successfull" << '\n';
    }

}

void test_mqtt_client()
{
    Imqtt_client *client = new mqtt_client();
    Config config;
    entity_parser parser;
    map<string, map<string, string>> table;
    config.read_ini_config_file("/home/champ/SecurityComponent/config/schedule.config", table);
    mqtt_entity entity = parser.get_mqtt_entity(table);
    client->start(entity);
}

void tls_server_check()
{
    tls_server server;
    entity_parser parser;
    Config config;
    map<string, map<string, string>> config_table;
    config.read_ini_config_file("/home/champ/SecurityComponent/config/agent.config", config_table);
    conn_entity entity = parser.get_conn_entity(config_table, "server");
    server.start(entity);
}

void tls_client_check()
{
    tls_client client;
    entity_parser parser;
    Config config;
    map<string, map<string, string>> config_table;
    config.read_ini_config_file("/home/champ/SecurityComponent/config/schedule.config", config_table);
    conn_entity entity = parser.get_conn_entity(config_table, "client");
    client.start(entity);
}

void test_tpm_controller()
{
    tpm_model model;
    model.host_address = "libtss2-tcti-mssim.so.0:host=127.0.0.1,port=2321";
    tpm_controller controller(model);
    controller.clear_tpm();

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
    test_hmac_signature();
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.close();
    }
    closelog();
    return 0;
}