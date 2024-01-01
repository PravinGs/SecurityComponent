#ifndef MCONTROLLER_HPP
#define MCONTROLLER_HPP
#pragma once

#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "model/entity_parser.hpp"
#include "service/monitor_service.hpp"


class monitor_controller
{
private:
    IMonitor * service = nullptr; 
    map<string, map<string, string>> config_table;
    Config config;
    entity_parser parser;
    bool is_valid_config;
    bool thread_handler;
    Proxy proxy;

public:

    monitor_controller(const map<string, map<string, string>> & config_table) : service(new monitor_service()), config_table(config_table), is_valid_config(true), thread_handler(true) {}

    monitor_controller(const string& config_file): service(new monitor_service()), thread_handler(true)
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false: true;
    }

    void start()
    {
        std::vector<string> processes{"tcp", "monitor"};

        std::vector<std::thread> threads(processes.size());

        for (int i = 0; i < (int)processes.size(); i++)
        {
            string process_name = processes[i];
            try
            {
                threads[i] = std::thread([&, process_name]()
                                         { assign_task_to_thread(process_name); });
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (auto &th : threads)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

     void assign_task_to_thread(const string &process)
    {
        if (process == "tcp")
        {
            std::cout << "This functionality not yet developed" << '\n';
        }
        else if (process == "monitor")
        {
             get_process_details();
        }
    }

    int get_process_details()
    {
        int result = SUCCESS;

        process_entity entity = parser.get_process_entity(config_table);

        if (!proxy.validate_process_entity(entity)) { return FAILED; }

        if (entity.time_pattern.empty())
        {
            return service->get_monitor_data(entity);
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
                agent_utils::write_log("Thread execution done.", DEBUG);
            }
            if (!thread_handler)
            {
                agent_utils::write_log("execution stopped from being runnig", DEBUG);
            }
        }
        catch(const std::exception& e)
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