#ifndef FIRMWARE_CONTROLLER
#define FIRMWARE_CONTROLLER

#include "service/patch_service.hpp"
#include "service/config_service.hpp"
#include "model/entity_parser.hpp"
#include "proxy/proxy.hpp"

#define DOWNLOAD_WAITING_TIME 5

class patch_controller
{
private:
    Ipatch_service *service = nullptr;
    Proxy proxy;
    Config config;
    entity_parser parser;
    map<string, map<string, string>> config_table;
    bool is_valid_config;

public:
    patch_controller(const map<string, map<string, string>> &config_table) : service(new patch_service()), config_table(config_table), is_valid_config(true) {}

    patch_controller(const string &config_file) : service(new patch_service())
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    int start()
    {

        if (!is_valid_config)
        {
            return FAILED;
        }

        patch_entity entity = parser.get_patch_entity(config_table);

        if (!proxy.validate_patch_entity(entity)) { return FAILED; }

        int result = service->start(entity);

        while (result == SERVER_ERROR)
        {

            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point executionTime = currentTime + std::chrono::seconds(DOWNLOAD_WAITING_TIME);
            std::chrono::system_clock::duration duration = executionTime - currentTime;
            int waitingTime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            std::this_thread::sleep_for(std::chrono::seconds(waitingTime));
            result = service->start(entity);
        }

        return result;
    }

    ~patch_controller()
    {
        delete service;
    }
};

#endif