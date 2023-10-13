#ifndef main_controller_HPP
#define main_controller_HPP

#include "controller/schedular_controller.hpp"
#include "service/apparmor_service.hpp"
#include "service/config_service.hpp"
#include "controller/mqtt_controller.hpp"

struct mqtt_connection_props;

const string SYSLOG  { "syslog"  };
const string APPLOG  { "applog"  };
const string PATCH   { "patch"   };
const string MONITOR { "monitor" };
const string MQTT    { "mqtt"   };
/**
 * @brief Main Controller
 *
 * The `main_controller` class serves as the central controller for the agent application. It is responsible for reading
 * the configuration file to initialize the application and start various activities. This controller acts as the
 * entry point for coordinating and managing the core functionalities of the agent.
 */
class main_controller
{
private:
    bool isReady = true; /**< A private variable for configuration file status*/
    Config _config; /**< A private instance of IniConfig for configuration management. */
    map<string, map<string, string>> _table; /**< A private map<string, map<string, string>> to store configuration data. */
    schedule *_schedule = nullptr; /**< A private instance of the schedule class. */
    apparmor_service a_service;
    mqtt_controller* m_controller = nullptr;
public:

    /**
     * @brief Main Controller Constructor
     *
     * The `main_controller` constructor initializes the global time for the agent, ensuring that the time reference is set
     * whenever an instance of this class is created. It also performs validation on the provided configuration file
     * (`configFile`) to ensure its validity for further use in the agent application.
     *
     * @param[in] configFile The path to the configuration file to be validated and used for agent settings.
     */
    main_controller(const string& configFile)
    {
        if (_config.read_ini_config_file(configFile, _table) != SUCCESS)
        {
            isReady = false;
        }
        if (isReady){
            const string scheduar_config_file = _table["schedule"]["config_file"];
            _schedule = new schedule(scheduar_config_file);
            m_controller = new mqtt_controller(_table);
        }
        auto today = std::chrono::system_clock::now();
        auto timeInfo = std::chrono::system_clock::to_time_t(today);
        std::tm *tm_info = std::localtime(&timeInfo);
        int day = tm_info->tm_mday;
        os::current_day = day; /* Current day at the application starting date. */
        os::current_month = tm_info->tm_mon;
        os::current_year = tm_info->tm_year+1900;
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
        // return ;
        if (!isReady)
            return;
        vector<std::thread> threads(3);
        vector<string> processes = {"schedule", "mqtt"};
        for (int i = 0; i < (int)processes.size(); i++)
        {
            try
            {
                string process_name = processes[i];
                threads[i] = std::thread([&, process_name]()
                                         { run(process_name); });

                agent_utils::write_log("[Agent] New thread creation for " + process_name, DEBUG);
            }
            catch (const std::exception &e)
            {
                string error = e.what();
                agent_utils::write_log(error, ERROR);
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
     * The `run` function takes the name of a controller, `process_name`, and initiates the corresponding controller's
     * activities. It is responsible for creating and managing the controller's thread and associated tasks.
     *
     * @param[in] process_name The name of the controller to run.
     */
    void run(const string& process_name)
    {
       
        if (process_name == "schedule")
        {
            //schedule schedule(_table[process_name]["config_file"]);
            _schedule->start();
        }
        else if (process_name == MQTT)
        {
            m_controller->start();
        }
    }

    /**
     * @brief Destructor for main_controller.
     *
     * The destructor performs cleanup tasks for the `main_controller` class.
     */
    ~main_controller()
    {
        delete _schedule;
    }
};
#endif