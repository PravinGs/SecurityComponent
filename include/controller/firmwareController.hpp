#ifndef FIRMWARE_CONTROLLER
#define FIRMWARE_CONTROLLER

#include "service/fwservice.hpp"
#include "proxy/Proxy.hpp"

#define DOWNLOAD_WAITING_TIME 5

/**
 * @brief Firmware Controller
 * 
 * The `FirmwareController` class serves as the controller layer for applications version management. 
 * It provides methods for initiating operations and managing related tasks.
 */
class FirmwareController
{
private:
    IFService *_fservice = nullptr; /**< A private pointer to the IFService service. */
    Proxy _proxy; /**< A private instance of the Proxy class. */
    const string firmware = "firmware"; /**< A private constant string for firmware component name. */
    
public:
    /**
     * @brief Construct a new Firmware Controller object
     * This constructor initializes the `FirmwareController` and creates an instance of the `FService`
     * to be used for firmware management.
     */
    FirmwareController() : _fservice(new FService()) {}

    /**
     * @brief Start Path Management Operation
     *
     * This function validates the configuration parameters provided in the `configTable` to ensure they meet the required
     * criteria for path management. After successful validation, it initiates the operation to manage paths based on the
     * validated configuration.
     *
     * @param[in] configTable A map containing configuration data for path management.
     *                       The map should be structured as follows:
     *                       - The keys are configuration identifiers.
     *                       - The values are maps containing path management settings.
     * @return An integer result code:
     *         - SUCCESS: The path management operation was successfully initiated.
     *         - FAILED: The validation or operation initiation encountered errors.
     */
    int start(map<string, map<string, string>> &configTable)
    {
        string application = configTable[firmware]["application"];
        string rootDir     = configTable[firmware]["root_dir"];
        if (application.empty()) //Precheck
        {
            AgentUtils::writeLog("No Application configured for patch management.", WARNING);
            return SUCCESS;
        }
        int result = _fservice->start(configTable);
        while (result == SERVER_ERROR)
        {

            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point executionTime = currentTime + std::chrono::seconds(DOWNLOAD_WAITING_TIME);
            std::chrono::system_clock::duration duration = executionTime - currentTime;
            int waitingTime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            std::this_thread::sleep_for(std::chrono::seconds(waitingTime));
            result = _fservice->download(configTable);
        }

        return result;
    }
    
    /**
     * @brief Destructor for FirmwareController.
     *
     * The destructor performs cleanup tasks for the `FirmwareController` class, which may include
     * releasing resources and deallocating memory, such as deleting the `_fservice` instance.
     */
    ~FirmwareController()
    {
        delete _fservice;
    }
};

#endif