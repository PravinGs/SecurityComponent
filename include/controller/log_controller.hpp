#ifndef log_controller_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "service/logcollector_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "service/curl_service.hpp"
#include "service/entity_parser.hpp"


/**
 * @brief Log Controller
 *
 * The `log_controller
 *` class serves as the controller layer for managing and reading logs from system and application logs.
 * It provides methods for initiating log reading operations and managing log-related tasks.
 */

class log_controller
{
private:
    map<string, map<string, string>> config_table;
    ILog * service = nullptr;                 /**< A private pointer to the ILog service. */
    Config config;                       /**< A private instance of IniConfig for configuration management. */
    entity_parser parser;
    bool is_valid_config = true;
    vector<std::future<int>> async_syslog_tasks; /**< A private vector of futures for asynchronous system tasks. */
    vector<std::future<int>> async_applog_tasks; /**< A private vector of futures for asynchronous application tasks. */
    Proxy proxy;                                 /**< A private instance of the Proxy class. */

public:

    /**
     * @brief Construct a new log_controller
     * object.
     *
     * This constructor initializes the `log_controller
     *` and creates an instance of the `LogService`
     * to be used for log management.
     */
    log_controller(const map<string, map<string, string>>& config_table) :  config_table(config_table), service(new log_service()) {}    

    log_controller(const string& config_file): service(new log_service())
    {
    
        if (config.read_ini_config_file(config_file, config_table) != SUCCESS)
        {
            is_valid_config = false;
        }

    
    }

    /**
     * @brief System Log Manager
     *
     * This function reads configured log files from the `config_table` and asynchronously invokes the `get_syslog` function
     * for each configured file. It manages the logging process and returns a result code.
     *
     * @param[in] config_table A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The logging process was successful for all configured files.
     *         - FAILED:  The logging process encountered errors for one or more files.
     * @see get_syslog
     */

    int syslog_manager()
    {

        if (!is_valid_config) return FAILED;

        log_entity entity = parser.get_log_entity(config_table, "syslog");
        
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
            auto asyncTask = [&, entity]() -> int
            {
                return get_syslog(entity);
            };
            async_syslog_tasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &async_task : async_syslog_tasks)
        {
            async_task.get();
        }

        return SUCCESS;
    }

    /**
     * @brief Get System Log
     *
     * This function performs necessary validations for a log file specified by `log_name` in the `config_table`. It reads
     * log data from the file located at `read_path` and processes it as needed.
     *
     * After validating, it invokes the `get_syslog` function from the ` service` instance to perform
     * the actual log collection. Finally, it sends the collected log data to the cloud for further processing.
     *
     * @param[in] config_table A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @param[in] read_path The path to the log file to be read.
     * @param[in] log_name The identifier of the log file to be processed.
     * @return An integer result code:
     *         - SUCCESS: The log file was successfully validated, read, and processed.
     *         - FAILED: The validation or processing of the log file encountered errors.
     */
    
    int get_syslog(const log_entity& e)
    {
        log_entity entity = const_cast<log_entity&>(e);
        int result = SUCCESS;

        if (!proxy.get_previous_log_read_time(entity)) { return FAILED; }

        if (!proxy.validate_log_entity(entity)) { return FAILED; }
        
        agent_utils::write_log("Reading " + entity.name + " starting...", INFO);
        result =  service->get_syslog(entity);
        if (result == SUCCESS) { agent_utils::update_log_written_time(entity.name, entity.current_read_time); }

        return result;
    }

    /**
     * @brief Application Log Manager
     *
     * This function reads configured log files from the `config_table` and asynchronously invokes the `get_applog` function
     * for each configured file. It manages the logging process and returns a result code.
     *
     * @param[in] config_table A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The logging process was successful for all configured files.
     *         - FAILED: The logging process encountered errors for one or more files.
     * 
     * @see get_applog
     */
    
    int applog_manager(map<string, map<string, string>> &config_table)
    {
        vector<string> apps = config.to_vector(config_table["applog"]["list"], ',');

        for (string app : apps)
        {
            auto asyncTask = [&, app]() -> int
            {
                return get_applog(config_table, app);
            }; 

            async_applog_tasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &asyncTask : async_applog_tasks)
        {
            asyncTask.get();
        }
        return SUCCESS;
    }

    /**
     * @brief Get Application Log
     *
     * This function performs necessary validations for a log file specified by `log_name` in the `config_table`. It reads
     * log data from the file located at `read_path` and processes it as needed.
     *
     * After validating, it invokes the `get_syslog` function from the ` service` instance to perform
     * the actual log collection. Finally, it sends the collected log data to the cloud for further processing.
     *
     * @param[in] config_table A map containing configuration data for log files.
     *                       The map should be structured as follows:
     *                       - The keys are log file identifiers.
     *                       - The values are maps containing log configuration settings.
     * @param[in] log_name The identifier of the log file to be processed.
     * @return An integer result code:
     *         - SUCCESS: The log file was successfully validated, read, and processed.
     *         - FAILED: The validation or processing of the log file encountered errors.
     */
 
    int get_applog(map<string, map<string, string>> &config_table, const string &log_name)
    {
        agent_utils::write_log("Reading " + log_name + " log starting...", DEBUG);
        long post_result = 0L;
        Json::Value json;
        char sep = ' ';
        const string write_path = config_table[log_name]["write_path"];
        const string read_path = config_table[log_name]["log_directory"];
        const string post_url = config_table["cloud"]["monitor_url"];
        const string form_name = config_table["cloud"]["form_name"];
        vector<string> attributes = config.to_vector(config_table[log_name]["columns"], ',');
        vector<string> log_levels = config.to_vector(config_table[log_name]["level"], ',');
        // string last_read_time = config.trim(proxy.getLastLogWrittenTime(config_table["cloud"]["name"], read_path));
        string last_read_time = "23232";

        if (proxy.isValidLogConfig(config_table, json, log_name, sep, last_read_time) == FAILED)
            return FAILED;

        if ( service->get_applog(json, attributes, read_path, write_path, last_read_time, log_levels, sep) == FAILED)
            return FAILED;

        post_result = curl_handler::post(post_url, form_name, write_path);

        if (post_result == POST_SUCCESS)
        {
            agent_utils::update_log_written_time(config_table[log_name]["last_time"], last_read_time);
            config.clean_file(write_path);
        }

        return (post_result == POST_SUCCESS) ? SUCCESS : FAILED;
    }

    /**
     * @brief Destructor for log_controller
     *.
     *
     * The destructor performs cleanup tasks for the `log_controller
     *` class, which may include
     * releasing resources and deallocating memory, such as deleting the ` service` instance.
     */
 
    virtual ~log_controller()
    {
        delete  service;
    }
};

#endif
