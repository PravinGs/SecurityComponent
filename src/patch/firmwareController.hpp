#ifndef FIRMWARE_CONTROLLER
#define FIRMWARE_CONTROLLER

#include "patch/fwservice.hpp"

class FirmWareController
{
private:
    IFService *_service = nullptr;
    const string firmware = "firmware";
    Proxy _proxy;

public:
    FirmWareController() : _service(new FService()) {}

    int start(map<string, map<string, string>> configTable)
    {
        if (configTable[firmware]["current_version"] == configTable[firmware]["latest_version"])
        {
            AgentUtils::writeLog("No Update Required");
            return SUCCESS;
        }
        int nextDownloadingTime = std::stoi(configTable["cloud"]["waiting_time"]);
        int result = _service->start(configTable);
        while (result == SERVER_ERROR)
        {

            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point executionTime = currentTime + std::chrono::seconds(nextDownloadingTime);
            std::chrono::system_clock::duration duration = executionTime - currentTime;
            int waitingTime = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            std::this_thread::sleep_for(std::chrono::seconds(waitingTime));
            result = _service->download(configTable);
        }

        return result;
    }
};

#endif