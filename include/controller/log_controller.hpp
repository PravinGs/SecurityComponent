#ifndef log_controller_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "service/logcollector_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "service/rest_service.hpp"
#include "model/entity_parser.hpp"

class log_controller
{
private:
    map<string, map<string, string>> config_table;
    ILog *service = nullptr;
    Config config;
    entity_parser parser;
    bool is_valid_config;
    bool thread_handler;
    vector<std::future<int>> async_syslog_tasks;
    vector<std::future<int>> async_applog_tasks;
    Proxy proxy;

public:
    log_controller(const map<string, map<string, string>> &config_table) : config_table(config_table), service(new log_service()), is_valid_config(true), thread_handler(true) {}

    log_controller(const string &config_file) : service(new log_service()), thread_handler(true)
    {
        is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false : true;
    }

    void start()
    {
        std::vector<string> processes{"tcp", "syslog", "applog"};

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
        else if (process == "applog")
        {
           applog_manager();
        }
        else if (process == "syslog")
        {
           syslog_manager();
        }
    }

    int syslog_manager()
    {
        int result = SUCCESS;

        if (!is_valid_config)
        {
            return FAILED;
        }

        log_entity entity = parser.get_log_entity(config_table, "syslog");

        if (entity.time_pattern.empty())
        {
            return create_syslog_async_tasks(entity);
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
                result = create_syslog_async_tasks(entity);
                if (result == FAILED)
                {
                    thread_handler = false;
                }
                agent_utils::write_log("log_controller: syslog_manager: thread execution done.", DEBUG);
            }
            if (!thread_handler)
            {
                agent_utils::write_log("log_controller: syslog_manager: execution stopped from being runnig", DEBUG);
            }
        }
        catch (std::exception &ex)
        {
            std::cerr << ex.what() << '\n';
        }
        return result;
    }

    int create_syslog_async_tasks(log_entity &entity)
    {
        vector<string> syslog_files = config.to_vector(entity.read_path, ',');
        for (const string &file : syslog_files)
        {
            string app_name;
            if (file.size() == 0)
                continue;

            size_t start = file.find_last_of('/');
            size_t end = file.find_first_of('.');

            if (start != std::string::npos && end != std::string::npos)
            {
                app_name = file.substr(start + 1, end - start - 1);
            }
            else if (start != std::string::npos && end == std::string::npos)
            {
                app_name = file.substr(start + 1);
            }
            else
            {
                break;
            }
            entity.read_path = file;
            entity.name = app_name;
            auto async_task = [&, entity]() -> int
            {
                return get_syslog(entity);
            };
            async_syslog_tasks.push_back(std::async(std::launch::async, async_task));
        }

        for (auto &async_task : async_syslog_tasks)
        {
            async_task.get();
        }
        return SUCCESS;
    }

    int get_syslog(const log_entity &e)
    {
        log_entity entity = const_cast<log_entity &>(e);
        int result = SUCCESS;

        if (!proxy.get_previous_log_read_time(entity))
        {
            return FAILED;
        }

        if (!proxy.validate_log_entity(entity))
        {
            return FAILED;
        }

        agent_utils::write_log("log_controller: get_syslog: reading " + entity.name + " starting...", INFO);
        result = service->get_syslog(entity);
        if (result == SUCCESS)
        {
            agent_utils::update_log_written_time(entity.name, entity.current_read_time);
        }

        return result;
    }

    int applog_manager()
    {
        vector<string> apps = config.to_vector(config_table["applog"]["list"], ',');

        for (string app : apps)
        {
            log_entity entity = parser.get_log_entity(config_table, app);
            entity.name = app;
            auto async_task = [&, entity]() -> int
            {
                return get_applog(entity);
            };

            async_applog_tasks.push_back(std::async(std::launch::async, async_task));
        }

        for (auto &async_task : async_applog_tasks)
        {
            async_task.get();
        }
        return SUCCESS;
    }

    int get_applog(const log_entity &e)
    {
        int result = SUCCESS;

        log_entity entity = const_cast<log_entity &>(e);

        if (!proxy.get_previous_log_read_time(entity))
        {
            return FAILED;
        }

        if (!proxy.validate_log_entity(entity))
        {
            return FAILED;
        }

        result = service->get_applog(entity);

        if (result == SUCCESS)
        {
            agent_utils::update_log_written_time(entity.name, entity.current_read_time);
        }
        return result;
    }

    virtual ~log_controller()
    {
        delete service;
    }
};

#endif
