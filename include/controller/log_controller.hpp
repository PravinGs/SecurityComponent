#ifndef log_controller_HPP
#define LOGCONTROLLET_HPP
#pragma once

#include "service/logcollector_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "service/curl_service.hpp"

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
    ILog *_log_service = nullptr;                 /**< A private pointer to the ILog service. */
    Config _config_service;                       /**< A private instance of IniConfig for configuration management. */
    vector<std::future<int>> _async_syslog_tasks; /**< A private vector of futures for asynchronous system tasks. */
    vector<std::future<int>> _async_applog_tasks; /**< A private vector of futures for asynchronous application tasks. */
    Proxy _proxy;                                 /**< A private instance of the Proxy class. */
    const string SYSLOG = "syslog";               /**< A private constant string for system log name. */

public:
    /**
     * @brief Construct a new log_controller
     * object.
     *
     * This constructor initializes the `log_controller
     *` and creates an instance of the `LogService`
     * to be used for log management.
     */
    log_controller() : _log_service(new log_service()) {}

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
     *         - FAILED: The logging process encountered errors for one or more files.
     *
     * @see get_syslog
     */
    int syslog_manager(map<string, map<string, string>> &config_table)
    {
        vector<string> syslogFiles = _config_service.to_vector(config_table["syslog"]["paths"], ',');

        for (const string &file : syslogFiles)
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
            auto asyncTask = [&, file, app_name]() -> int
            {
                return get_syslog(config_table, file, app_name);
            };
            _async_syslog_tasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &async_task : _async_syslog_tasks)
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
     * After validating, it invokes the `get_syslog` function from the `_log_service` instance to perform
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
    int get_syslog(map<string, map<string, string>> &config_table, const string &read_path, const string &log_name)
    {
        int result;
        agent_utils::write_log("Reading " + log_name + " starting...", INFO);
        Json::Value json;
        char remote;
        const string post_url = config_table["cloud"]["log_url"];
        const string name = config_table["cloud"]["form_name"];
        string last_read_time = _config_service.trim(_proxy.getLastLogWrittenTime(log_name, read_path));
        vector<string> attributes = _config_service.to_vector(config_table[SYSLOG]["columns"], ',');
        vector<string> log_levels = _config_service.to_vector(config_table[SYSLOG]["level"], ',');

        result = _proxy.isValidLogConfig(config_table, json, SYSLOG, remote, last_read_time);

        if (result == FAILED)
            return result;

        json["app_name"] = log_name;
        result = _log_service->get_syslog(log_name, json, attributes, read_path, last_read_time, log_levels, remote);
        if (result == SUCCESS)
            agent_utils::update_log_written_time(log_name, last_read_time);

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
        vector<string> apps = _config_service.to_vector(config_table["applog"]["list"], ',');

        for (string app : apps)
        {

            auto asyncTask = [&, app]() -> int
            {
                return get_applog(config_table, app);
            }; 

            _async_applog_tasks.push_back(std::async(std::launch::async, asyncTask));
        }

        for (auto &asyncTask : _async_applog_tasks)
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
     * After validating, it invokes the `get_syslog` function from the `_log_service` instance to perform
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
        vector<string> attributes = _config_service.to_vector(config_table[log_name]["columns"], ',');
        vector<string> log_levels = _config_service.to_vector(config_table[log_name]["level"], ',');
        string last_read_time = _config_service.trim(_proxy.getLastLogWrittenTime(config_table["cloud"]["name"], read_path));

        if (_proxy.isValidLogConfig(config_table, json, log_name, sep, last_read_time) == FAILED)
            return FAILED;

        if (_log_service->get_applog(json, attributes, read_path, write_path, last_read_time, log_levels, sep) == FAILED)
            return FAILED;

        post_result = curl_handler::post(post_url, form_name, write_path);

        if (post_result == POST_SUCCESS)
        {
            agent_utils::update_log_written_time(config_table[log_name]["last_time"], last_read_time);
            _config_service.clean_file(write_path);
        }

        return (post_result == POST_SUCCESS) ? SUCCESS : FAILED;
    }

    /**
     * @brief Destructor for log_controller
     *.
     *
     * The destructor performs cleanup tasks for the `log_controller
     *` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_log_service` instance.
     */
    virtual ~log_controller()
    {
        delete _log_service;
    }
};

#endif
