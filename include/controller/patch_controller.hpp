#ifndef FIRMWARE_CONTROLLER
#define FIRMWARE_CONTROLLER

#include "service/patch_service.hpp"
#include "proxy/proxy.hpp"

#define DOWNLOAD_WAITING_TIME 5

/**
 * @brief Firmware Controller
 * 
 * The `patch_controller` class serves as the controller layer for applications version management. 
 * It provides methods for initiating operations and managing related tasks.
 */
class patch_controller
{
private:
    Ipatch_service *_patch_service = nullptr; /**< A private pointer to the Ipatch_service service. */
    Proxy _proxy; /**< A private instance of the Proxy class. */
    const string firmware = "firmware"; /**< A private constant string for firmware component name. */
    
public:
    /**
     * @brief Construct a new Firmware Controller object
     * This constructor initializes the `patch_controller` and creates an instance of the `patch_service`
     * to be used for firmware management.
     */
    patch_controller() : _patch_service(new patch_service()) {}

    /**
     * @brief Start Path Management Operation
     *
     * This function validates the configuration parameters provided in the `config_table` to ensure they meet the required
     * criteria for path management. After successful validation, it initiates the operation to manage paths based on the
     * validated configuration.
     *
     * @param[in] config_table A map containing configuration data for path management.
     *                       The map should be structured as follows:
     *                       - The keys are configuration identifiers.
     *                       - The values are maps containing path management settings.
     * @return An integer result code:
     *         - SUCCESS: The path management operation was successfully initiated.
     *         - FAILED: The validation or operation initiation encountered errors.
     */
    int start(map<string, map<string, string>> &config_table)
    {
        string application = config_table[firmware]["application"];
        string rootDir     = config_table[firmware]["root_dir"];
        if (application.empty()) //Precheck
        {
            agent_utils::write_log("No Application configured for patch management.", WARNING);
            return SUCCESS;
        }
        int result = _patch_service->start(config_table);
        while (result == SERVER_ERROR)
        {

            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point executionTime = currentTime + std::chrono::seconds(DOWNLOAD_WAITING_TIME);
            std::chrono::system_clock::duration duration = executionTime - currentTime;
            int waitingTime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            std::this_thread::sleep_for(std::chrono::seconds(waitingTime));
            result = _patch_service->start(config_table);
        }

        return result;
    }
    
    /**
     * @brief Destructor for patch_controller.
     *
     * The destructor performs cleanup tasks for the `patch_controller` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_patch_service` instance.
     */
    ~patch_controller()
    {
        delete _patch_service;
    }
};

#endif