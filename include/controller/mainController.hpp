#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

#include "controller/scheduleController.hpp"
#include "controller/watchFileController.hpp"
#include "service/configservice.hpp"
#include "service/connection.hpp"

/**
 * @brief Main Controller
 *
 * The `MainController` class serves as the central controller for the agent application. It is responsible for reading
 * the configuration file to initialize the application and start various activities. This controller acts as the
 * entry point for coordinating and managing the core functionalities of the agent.
 */
class MainController
{
private:
    bool isReady = true; /**< A private variable for configuration file status*/
    Config _config; /**< A private instance of IniConfig for configuration management. */
    map<string, map<string, string>> _table; /**< A private map<string, map<string, string>> to store configuration data. */
    Schedule schedule; /**< A private instance of the Schedule class. */

public:

    /**
     * @brief Main Controller Constructor
     *
     * The `MainController` constructor initializes the global time for the agent, ensuring that the time reference is set
     * whenever an instance of this class is created. It also performs validation on the provided configuration file
     * (`configFile`) to ensure its validity for further use in the agent application.
     *
     * @param[in] configFile The path to the configuration file to be validated and used for agent settings.
     */
    MainController(const string& configFile)
    {
        if (_config.readIniConfigFile(configFile, _table) != SUCCESS)
        {
            isReady = false;
        }
        auto today = std::chrono::system_clock::now();
        auto timeInfo = std::chrono::system_clock::to_time_t(today);
        std::tm *tm_info = std::localtime(&timeInfo);
        int day = tm_info->tm_mday;
        OS::GlobalDay = day; /* Current day at the application starting date. */
    }

    /**
     * @brief Start Agent Activities
     *
     * The `start` function creates threads and maps them to their respective controllers based on their names. The number
     * of threads is determined by the sections specified in the agent's configuration file, which is in INI format.
     * Each section corresponds to a controller responsible for specific activities. Controller mapping is achieved by
     * invoking the `run` function for each controller name found in the configuration.
     *
     * This function serves as the entry point for initializing and orchestrating the various activities of the agent
     * application in parallel.
     */
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
                cerr << e.what() << "\n";
            }
        }

        for (auto &thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }

    /**
     * @brief Run Controller by Name
     *
     * The `run` function takes the name of a controller, `processName`, and initiates the corresponding controller's
     * activities. It is responsible for creating and managing the controller's thread and associated tasks.
     *
     * @param[in] processName The name of the controller to run.
     */
    void run(const string& processName)
    {
        if (processName == "schedule")
        {
            //Schedule schedule(_table[processName]["config_file"]);
            schedule.start();
        }
        /*else if (processName == "watcher")
        {
            WatchController watcher(_table[processName]["watch_dir"], _table[processName]["backup_dir"]);
            watcher.start();
        }
        else if (processName == "tls")
        {
            TlsConnection connection(_table[processName]["port"], _table[processName]["ca_pem"], _table[processName]["server_cert"], _table[processName]["server_key"]);
            connection.start();
        }
        else
        {
            AgentUtils::writeLog("Invalid Process Name", FAILED);
        }
        else if (processName = "tls") {cout << "Does not implemented yet" << endl;}
        */
    }

    /**
     * @brief Destructor for MainController.
     *
     * The destructor performs cleanup tasks for the `MainController` class.
     */
    ~MainController()
    {
        
    }
};

#endif