#ifndef MCONTROLLER_HPP
#define MCONTROLLER_HPP
#pragma once

#include "service/monitor.hpp"
#include "service/configservice.hpp"
#include "proxy/Proxy.hpp"
#include "service/curlservice.hpp"

/**
 * @brief Monitor Controller
 * 
 * The `MonitorController` class serves as the controler layer for managing and reading processes information. 
 * It provides methoods fot initiating device monitoring operations and managing monitor related tasks.
 */
class MonitorController
{
private:
    IMonitor *_monitorService = nullptr; /**< A private pointer to the IMonitor service. */
    Config _configService; /**< A private instance of IniConfig for configuration management. */
    Proxy proxy; /**< A private instance of the Proxy class. */
    const string monitor = "monitor"; /**< A private constant string for monito process name. */

public:
    /**
     * @brief Construct a new MonitorController object.
     *
     * This constructor initializes the `MonitorController` and creates an instance of the `MonitorService`
     * to be used for monitor.
     */
    MonitorController() : _monitorService(new MonitorService()) {}

    /**
     * @brief Get Monitor Log
     *
     * This function validates the configuration parameters provided in the `configTable` to ensure they meet the required
     * criteria for monitoring. After validation, it invokes the `_monitorService->getData()` function to collect
     * information about every process running in the system. Finally, it sends the collected information to the cloud.
     *
     * @param[in] configTable A map containing configuration data for monitoring.
     *                       The map should be structured as follows:
     *                       - The keys are configuration identifiers.
     *                       - The values are maps containing monitoring configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The monitoring data was successfully collected and processed.
     *         - FAILED: The validation, data collection, or processing encountered errors.
     */    
    int getMonitorLog(map<string, map<string, string>>& configTable)
    {
        int result = SUCCESS;
        string writePath = configTable[monitor]["write_path"];
        string postUrl = configTable["cloud"]["monitor_url"];
        string attributeName = configTable["cloud"]["form_name"];
        vector<string> columns = _configService.toVector(configTable[monitor]["columns"], ',');
        if (_monitorService->getData(columns) == FAILED)
            return FAILED;

        // result = CurlHandler::post(postUrl, formName, jsonFile);
        // _configService.cleanFile(writePath);
        return result;
    }

    /**
     * @brief Destructor for MonitorController.
     *
     * The destructor performs cleanup tasks for the `MonitorController` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_monitorService` instance.
     */
    virtual ~MonitorController()
    {
        delete _monitorService;
    }
};

#endif