#pragma once

#include "agent_utils.hpp"
#include "service/log_analysis_service.hpp"
#include "service/config_service.hpp"
#include "proxy/proxy.hpp"
#include "model/entity.hpp"
#include "model/entity_parser.hpp"


class analysis_controller
{
    private:
        I_analysis * analysis = nullptr; /**< A private pointer to the log_analysis service. */
        Config config;
        Proxy proxy;
        entity_parser parser;
        map<string, map<string,string>> config_table;
        bool is_valid_config;
        
    public: 

        analysis_controller(const map<string, map<string,string>>& config_table) : analysis(new log_analysis()), config_table(config_table), is_valid_config(true) {} 

        analysis_controller(const string& config_file) : analysis(new log_analysis())
        {
            is_valid_config = (config.read_ini_config_file(config_file, config_table) != SUCCESS) ? false: true;
        }

        void start()
        {
            std::vector<string> processes{"tcp", "analysis"};

            std::vector<std::thread> threads(processes.size());

            for (int i = 0; i < (int)processes.size(); i++)
            {
                string process_name = processes[i];
                try
                {
                    threads[i] = std::thread([&, process_name]()
                                            { assign_task_to_thread(process_name); });
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (auto &th : threads)
            {
                if (th.joinable())
                {
                    th.join();
                }
            }
        }

        void assign_task_to_thread(const string &process)
        {
            if (process == "tcp")
            {
                std::cout << "This functionality not yet developed" << '\n';
            }
            else if (process == "analysis")
            {
                cout << "Log analysis result : " << '\n';
            }

        }

        int  analyse()
        {
            analysis_entity entity = parser.get_analysis_entity(config_table);

            if (!proxy.validate_analysis_entity(entity)) { return FAILED; }

            if (entity.time_pattern.empty())
            {
                return analysis->start(entity);
            }

            // here scheduling code willbe applied 

            int result = analysis->start(entity);

            return result;
        }

        virtual ~analysis_controller() {delete analysis;}

};