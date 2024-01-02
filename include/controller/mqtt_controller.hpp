#ifndef MQTT_CLIENT
#define MQTT_CLIENT

#include "service/mqtt_service.hpp"
#include "service/config_service.hpp"
#include "model/entity_parser.hpp"
#include "proxy/proxy.hpp"

class mqtt_controller
{
private:
    Imqtt_client * client = nullptr;
    Config config;
    entity_parser parser;
    Proxy proxy;
    map<string, map<string, string>> config_table;
    bool is_valid_config;

public:

    mqtt_controller(const map<string, map<string, string>>& config_table): client(new mqtt_client()), config_table(config_table), is_valid_config(true) {}

    mqtt_controller(const string& config_file): client(new mqtt_client())
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    void start()
    {
         if (!is_valid_config)
        {
            return FAILED;
        }
        mqtt_entity entity = parser.get_mqtt_entity(config_table);
        if (!proxy.validate_mqtt_entity(entity)) { return; }

        client->start(entity);
    }

    ~mqtt_controller()
    {
        delete client;
    }

};

#endif

