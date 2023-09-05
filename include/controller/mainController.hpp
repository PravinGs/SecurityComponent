#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

#include "controller/scheduleController.hpp"
#include "controller/watchFileController.hpp"
#include "service/configservice.hpp"
#include "service/connection.hpp"

class MainController
{
private:
    bool isReady = true;
    IniConfig _config;
    map<string, map<string, string>> _table;

public:
    MainController(const string configFile)
    {
        if (_config.readConfigFile(configFile, _table) != SUCCESS)
        {
            isReady = false;
        }
        auto today = std::chrono::system_clock::now();
        auto timeInfo = std::chrono::system_clock::to_time_t(today);
        std::tm *tm_info = std::localtime(&timeInfo);
        int day = tm_info->tm_mday;
        OS::GlobalDay = day; /* Current day at the application starting date. */
    }

    void start()
    {
        if (!isReady)
            return;
        vector<std::thread> threads(3);
        vector<string> processes = {"schedule", "watcher", "tls"};
        for (int i = 0; i < (int)processes.size(); i++)
        {
            try
            {
                string processName = processes[i];
                threads[i] = std::thread([&, processName]()
                                         { run(processName); });
            }
            catch (const std::exception &e)
            {
                cerr << e.what() << endl;
            }
        }

        for (auto &thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }

    void run(const string processName)
    {
        if (processName == "schedule")
        {
            Schedule schedule(_table[processName]["config_file"]);
            schedule.start();
        }
        // else if (processName == "watcher")
        // {
        //     WatchController watcher(_table[processName]["watch_dir"], _table[processName]["backup_dir"]);
        //     watcher.start();
        // }
        // else if (processName == "tls")
        // {
        //     TlsConnection connection(_table[processName]["port"], _table[processName]["ca_pem"], _table[processName]["server_cert"], _table[processName]["server_key"]);
        //     connection.start();
        // }
        // else
        // {
        //     AgentUtils::writeLog("Invalid Process Name", FAILED);
        // }
        // else if (processName = "tls") {cout << "Does not implemented yet" << endl;}
    }
};

#endif