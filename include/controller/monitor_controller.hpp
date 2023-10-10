#ifndef MCONTROLLER_HPP
#define MCONTROLLER_HPP
#pragma once

#include "service/monitor_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "service/curl_service.hpp"

/**
 * @brief Monitor Controller
 * 
 * The `monitor_controller` class serves as the controler layer for managing and reading processes information. 
 * It provides methoods fot initiating device monitoring operations and managing monitor related tasks.
 */
class monitor_controller
{
private:
    IMonitor *_monitor_service = nullptr; /**< A private pointer to the IMonitor service. */
    Config _config_service; /**< A private instance of IniConfig for configuration management. */
    Proxy proxy; /**< A private instance of the Proxy class. */
    const string monitor = "monitor"; /**< A private constant string for monito process name. */

public:
    /**
     * @brief Construct a new monitor_controller object.
     *
     * This constructor initializes the `monitor_controller` and creates an instance of the `monitor_service`
     * to be used for monitor.
     */
    monitor_controller() : _monitor_service(new monitor_service()) {}

    /**
     * @brief Get Monitor Log
     *
     * This function validates the configuration parameters provided in the `config_table` to ensure they meet the required
     * criteria for monitoring. After validation, it invokes the `_monitor_service->get_monitor_data()` function to collect
     * information about every process running in the system. Finally, it sends the collected information to the cloud.
     *
     * @param[in] config_table A map containing configuration data for monitoring.
     *                       The map should be structured as follows:
     *                       - The keys are configuration identifiers.
     *                       - The values are maps containing monitoring configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The monitoring data was successfully collected and processed.
     *         - FAILED: The validation, data collection, or processing encountered errors.
     */    
    int getMonitorLog(map<string, map<string, string>>& config_table)
    {
        int result = SUCCESS;
        string writePath = config_table[monitor]["write_path"];
        string postUrl = config_table["cloud"]["monitor_url"];
        string attributeName = config_table["cloud"]["form_name"];

        if (_monitor_service->get_monitor_data() == FAILED)
            return FAILED;

        // result = curl_handler::post(postUrl, formName, jsonFile);
        // _config_service.clean_file(writePath);
        return result;
    }

    /**
     * @brief Destructor for monitor_controller.
     *
     * The destructor performs cleanup tasks for the `monitor_controller` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_monitor_service` instance.
     */
    virtual ~monitor_controller()
    {
        delete _monitor_service;
    }
};

#endif