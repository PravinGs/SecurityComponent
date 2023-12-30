#ifndef SCHEDULAR_HPP
#define SCHEDULAR_HPP

#pragma once

#include "controller/log_controller.hpp"
#include "controller/monitor_controller.hpp"
#include "controller/patch_controller.hpp"
#include "service/config_service.hpp"

/**
 * @brief schedule Manager
 *
 * The `schedule` class is responsible for managing activities defined in a scheduler configuration file. It executes
 * these activities based on the time patterns defined in the configuration. This class serves as the core component
 * for scheduling and automating tasks or activities in your application.
 */
class schedule
{

private:
    log_controller * _log_controller = nullptr;         /**< A private instance of the log_controller class. */
    monitor_controller _monitor_controller; /**< A private instance of the monitor_controller class. */
    patch_controller _patch_controller;      /**< A private instance of the patch_controller class. */
    Config _config_service;                /**< A private instance of the IniConfig class. */
    map<string, map<string, string>> _config_table;
    bool _is_ready_to_schedule = true; /**< A private variable for configuration file status*/

private:
    /**
     * @brief Run scheduled Process
     *
     * The `run` function maps a process to its corresponding controller based on `process_name` and executes the task
     * defined by the process at the specified `time_pattern`. If any process returns a failure result, the `index` is used
     * to indicate the corresponding thread index in the `process_status` vector.
     *
     * @param[in] process_name The name of the process to be executed.
     * @param[in] time_pattern The time pattern defining when the process should be executed.
     * @param[in] index The thread index associated with the process in the `process_status` vector if the process returns a failure result.
     * @param[out] process_status A vector containing thread status information.
     */
    void run(const string &process_name, const string &time_pattern, int index, vector<bool> &process_status);
    void print_time(std::chrono::system_clock::time_point &t);

    /**
     * @brief Process Time Pattern
     *
     * The `process_time_pattern` function converts a string-formatted time pattern into a usable `pattern_table` that contains
     * values for seconds, minutes, and hours. This conversion allows the application to work with and interpret time patterns
     * specified in a human-readable format.
     *
     * @param[out] pattern_table A vector to store the time pattern values (seconds, minutes, hours).
     * @param[in] pattern The string-formatted time pattern to be processed and converted.
     * @return An integer result code:
     *         - SUCCESS: The time pattern was successfully processed and converted.
     *         - FAILED: The processing encountered errors or an invalid time pattern format.
     */
    int process_time_pattern(vector<int> &pattern_table, const string &pattern);
    // void task(const string &process_name, std::vector<bool> &process_status, int index, log_controller &_log_controller, monitor_controller &_monitor_controller, patch_controller &_patch_controller, IniConfig &_config_service, map<string, map<string, string>> &_config_table);

public:
    schedule() = default;
    /**
     * @brief schedule Manager Constructor
     *
     * The `schedule` class constructor initializes the schedule manager using a configuration file. This file contains
     * information about scheduled activities and their associated time patterns.
     *
     * @param[in] file The path to the configuration file that defines scheduled activities and time patterns.
     */
    schedule(const string &file);

    /**
     * @brief Start scheduled Activities
     *
     * The `start` function creates threads and maps them to their respective controllers based on their names. The number
     * of threads is determined by the sections specified in the scheduler configuration file, which is in INI format.
     * Each section corresponds to a controller responsible for specific activities. Controller mapping is achieved by
     * invoking the `run` function for each controller name found in the configuration.
     *
     * This function serves as the entry point for initializing and orchestrating the various activities of the agent
     * application in parallel.
     */
    void start();
    /**
     * @brief Destructor for schedule.
     *
     * The destructor performs cleanup tasks for the `schedule` class.
     */
    virtual ~schedule();
};

schedule::schedule(const string &file)
{
    cout << "Schedular configuration file : " << file << "\n";
    if (_config_service.read_ini_config_file(file, _config_table) != SUCCESS)
    {
        _is_ready_to_schedule = false;
    }
    else
    {
        _log_controller = new log_controller(_config_table);
    }
}

void print_next_execution_time(std::tm *next_time_info)
{
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", next_time_info);
    std::string next_time_str(buffer);
    agent_utils::write_log("Next execution time: " + next_time_str, DEBUG);
}

void print_duration(const std::chrono::duration<double> &duration)
{
    int hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    auto remaining_duration = duration - std::chrono::hours(hours);
    int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining_duration).count();
    remaining_duration -= std::chrono::minutes(minutes);
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining_duration).count();
    agent_utils::write_log("Duration until next execution: " + std::to_string(hours) + " hours, " + std::to_string(minutes) + " minutes, " + std::to_string(seconds) + " seconds.", DEBUG);
}

void schedule::run(const string &process_name, const string &time_pattern, int index, vector<bool> &process_status)
{
    try
    {
        auto cron = cron::make_cron(time_pattern);
        while (process_status[index])
        {
            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(currentTime);
            std::time_t next = cron::cron_next(cron, time);
            std::chrono::system_clock::time_point targetPoint = std::chrono::system_clock::from_time_t(next);
            std::tm *next_time_info = std::localtime(&next);
            if (agent_utils::debug)
            {
                print_next_execution_time(next_time_info);
            }
                
            std::chrono::duration<double> duration = targetPoint - currentTime;
            
            if (agent_utils::debug){
                print_duration(duration);
            }

            std::this_thread::sleep_for(duration);
            if (process_status[index])
            {
                if (strcmp(process_name.c_str(), "monitor") == 0)
                {
                    if (_monitor_controller.getMonitorLog(_config_table) == SUCCESS)
                    {
                        if (agent_utils::debug){
                            agent_utils::write_log("Monitor Log collected successfully.", DEBUG);
                        }  
                    }
                    else
                    {
                        if (agent_utils::debug){
                            agent_utils::write_log("Reading Process details operation stopped", DEBUG);
                        }
                        
                        process_status[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(process_name.c_str(), "applog") == 0)
                {
                    if (_log_controller->applog_manager() == SUCCESS)
                    {
                        if (agent_utils::debug){
                            agent_utils::write_log("Applog operation done.", DEBUG);
                        }                        
                    }
                    else
                    {
                        if(agent_utils::debug){
                            agent_utils::write_log("Reading AppLog process stopped", DEBUG);
                        }
                        process_status[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(process_name.c_str(), "syslog") == 0)
                {
                    if (_log_controller->syslog_manager() == SUCCESS)
                    {
                        std::cout << "Syslog operation done"
                                  << "\n";
                    }
                    else
                    {
                        agent_utils::write_log("Reading SysLog process stopped", DEBUG);
                        process_status[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
                else if (strcmp(process_name.c_str(), "firmware") == 0)
                {
                    int response = _patch_controller.start(_config_table);
                    if (response == SUCCESS)
                    {
                        std::cout << "FirmWare operation done"
                                  << "\n";
                    }
                    else if (response == SCHEDULAR_WAIT)
                    {
                        int timeoutMinutes = 30;
                        std::chrono::minutes timeoutDuration(timeoutMinutes);
                        std::this_thread::sleep_for(timeoutDuration);
                    }
                    else
                    {
                        /*Do something*/
                        process_status[index] = false; // Mark the process as failed
                        break;                        // Exit the loop immediately
                    }
                }
            }
            agent_utils::write_log(process_name + " execution done.", DEBUG);

        } // end of while loop
        // Additional check after the loop to write the log message if the process failed
        if (!process_status[index])
        {
            agent_utils::write_log(process_name + " execution stopped from being runnig", FAILED);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void schedule::print_time(std::chrono::system_clock::time_point &t)
{
    std::time_t currentTime = std::chrono::system_clock::to_time_t(t);
    struct std::tm *now_tm = std::localtime(&currentTime);

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now_tm);
    cout << buffer << "\n";
}

int schedule::process_time_pattern(vector<int> &pattern_table, const string &pattern)
{
    int result = SUCCESS;
    if (pattern.length() == 0)
    {
        agent_utils::write_log("Invalid Pattern found [ " + pattern + " ]", FAILED);
        return FAILED;
    }
    int position = 0;
    string token;
    std::istringstream iss(pattern);
    while (std::getline(iss, token, ' '))
    {
        int processed_token;
        try
        {
            if (token.length() == 1)
            {
                if (token[0] == '*')
                    processed_token = 1;
                else
                {
                    processed_token = std::stoi(token);
                }
                pattern_table.push_back(processed_token);
                position++;
                continue;
            }

            else
            {
                token = token.substr(2);
            }
            processed_token = std::stoi(token);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            string err = e.what();
            agent_utils::write_log("Invalid Pattern" + err, FAILED);
            result = FAILED;
            break;
        }
        pattern_table.push_back(processed_token);
        position++;
    }
    return result;
}

void schedule::start()
{
    if (!_is_ready_to_schedule)
        return;

    std::vector<std::string> processes;
    std::map<std::string, std::string> schedular = _config_table["schedular"];
    agent_utils::write_log("Schedular started", DEBUG);
    for (const auto &process : schedular)
    {
        processes.push_back(process.first);
    }

    std::vector<std::thread> threads(processes.size());

    std::vector<bool> process_status(processes.size(), true); // Track the status of each process

    for (int i = 0; i < (int)processes.size(); i++)
    {
        try
        {
            std::string process_name = processes[i];
            std::string process_time_pattern = schedular[processes[i]];
            //            std::cout << process_name << " : " << process_time_pattern << "\n";
            threads[i] = std::thread([&, process_name, process_time_pattern]()
                                     {
                if (process_status[i]) 
                {
                    run(process_name, process_time_pattern, i, process_status); 
                } });
            agent_utils::write_log("[Schedular] New thread creation for " + process_name, DEBUG);
        }
        catch (const std::exception &e)
        {
            // std::cerr << e.what() << "\n";
            string error = e.what();
            agent_utils::write_log(error, FAILED);
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for tasks to start

    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    agent_utils::write_log("Schedular function finished", DEBUG);
}

schedule::~schedule() {}

#endif