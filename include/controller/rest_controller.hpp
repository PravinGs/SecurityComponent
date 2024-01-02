#ifndef REST_CONTROLLER
#define REST_CONTROLLER

#include "service/rest_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "model/entity_parser.hpp"

class rest_controller
{
private:
    rest_service *service = nullptr;
    Proxy proxy;
    Config config;
    entity_parser parser;
    map<string, map<string, string>> config_table;
    bool is_valid_config;

public:
    patch_controller(const map<string, map<string, string>> &config_table) : service(new rest_service()), config_table(config_table), is_valid_config(true) {}

    patch_controller(const string &config_file) : service(new rest_service())
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    void start()
    {
        if (!is_valid_config)
        {
            return;
        }

        rest_entity entity = parser.get_rest_entity(config_table);

        if (!proxy.validate_rest_entity(entity)) {return; }
        
    }

    ~rest_controller()
    {
        delete service;
    }
};

#endif