#pragma once

#include "agent_utils.hpp"
#include "service/log_analysis_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "model/entity.hpp"
#include "model/entity_parser.hpp"
#include "comm/client.hpp"

class analysis_controller
{
private:
    I_analysis *analysis = nullptr; /**< A private pointer to the log_analysis service. */
    tls_client client;
    Config config;
    Proxy proxy;
    entity_parser parser;
    map<string, map<string, string>> config_table;
    bool is_valid_config;
    bool thread_handler;
    int client_connection;

public:
    analysis_controller(const map<string, map<string, string>> &config_table) : analysis(new log_analysis()), config_table(config_table), is_valid_config(true), thread_handler(true), client_connection(0) {}

    analysis_controller(const string &config_file) : analysis(new log_analysis()), thread_handler(true), client_connection(0)
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    int start()
    {
        int result = SUCCESS;

        if (!is_valid_config)
        {
            return FAILED;
        }

        analysis_entity entity = parser.get_analysis_entity(config_table);

        client_connection = client.start(entity.connection);

        if (client_connection == FAILED)
        {
            agent_utils::write_log("analysis_controller: start: tls connection to the host application failed", WARNING);
        }

        if (!proxy.validate_analysis_entity(entity))
        {
            return FAILED;
        }

        if (entity.time_pattern.empty())
        {
            return analysis->start(entity);
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
                result = analysis->start(entity);
                if (result == FAILED)
                {
                    thread_handler = false;
                }
                else
                {
                    client_data data = connection::build_client_data("analysis", "publish");
                    client.send_client_data(data); // handle have to updated for data not being send to host applicaiton.
                }
                agent_utils::write_log("analysis_controller: start: thread execution done.", DEBUG);
            }
            if (!thread_handler)
            {
                agent_utils::write_log("analysis_controller: start: execution stopped from being runnig", DEBUG);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        int result = analysis->start(entity);

        return result;
    }

    virtual ~analysis_controller() { delete analysis; }
};