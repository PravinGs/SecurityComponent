#ifndef MCONTROLLER_HPP
#define MCONTROLLER_HPP
#pragma once

#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "model/entity_parser.hpp"
#include "service/monitor_service.hpp"
#include "comm/client.hpp"

class monitor_controller
{
private:
    IMonitor *service = nullptr;
    map<string, map<string, string>> config_table;
    tls_client client;
    Config config;
    entity_parser parser;
    bool is_valid_config;
    bool thread_handler;
    Proxy proxy;
    int client_connection;

public:
    monitor_controller(const map<string, map<string, string>> &config_table) : service(new monitor_service()), config_table(config_table), is_valid_config(true), thread_handler(true), client_connection(0) {}

    monitor_controller(const string &config_file) : service(new monitor_service()), thread_handler(true), client_connection(0)
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    int start()
    {
        int result = SUCCESS;

        if (!is_valid_config) { return FAILED; }

        process_entity entity = parser.get_process_entity(config_table);

        client_connection = client.start(entity.connection);

        if (client_connection == FAILED)
        {
           agent_utils::write_log("monitor_controller: get_process_details: tls connection to host application failes", WARNING);
        }

        if (!proxy.validate_process_entity(entity))
        {
            return FAILED;
        }

        if (entity.time_pattern.empty())
        {
            result = service->get_monitor_data(entity);
            if (result != FAILED && client_connection == SUCCESS)
            {
                client_data data = connection::build_client_data("process", "publish");
                client.send_client_data(data);
            }
            return result;
        }

        try
        {
            auto cron = cron::make_cron(entity.time_pattern);
            while (thread_handler)
            {
                std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
                std::time_t time = std::chrono::system_clock::to_time_t(current_time);
                std::time_t next = cron::cron_next(cron, time);
                std::chrono::system_clock::time_point target_point = std::chrono::system_clock::from_time_t(next);
                std::tm *next_time_info = std::localtime(&next);
                agent_utils::print_next_execution_time(next_time_info);
                std::chrono::duration<double> duration = target_point - current_time;
                agent_utils::print_duration(duration);
                std::this_thread::sleep_for(duration);
                result = service->get_monitor_data(entity);
                if (result == FAILED)
                {
                    thread_handler = false;
                }
                else
                {
                    client_data data = connection::build_client_data("analysis", "publish");
                    client.send_client_data(data); // handle have to updated for data not being send to host applicaiton.
                }
                agent_utils::write_log("monitor_controller: get_process_details: thread execution done.", DEBUG);
            }
            if (!thread_handler)
            {
                agent_utils::write_log("monitor_controller: get_process_details: execution stopped from being runnig", DEBUG);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        return result;
    }

    virtual ~monitor_controller()
    {
        delete service;
    }
};

#endif