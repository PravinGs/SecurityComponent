#pragma once
#include "common.hpp"
#include "base_api.hpp"
#include "service/config_service.hpp"
#include "entity/agent_entity.hpp"
#include "iservice/log_api.hpp"
#include "iservice/analaysis_api.hpp"
#include "iservice/ids_api.hpp"
#include "iservice/process_api.hpp"

class agent
{
public:
    agent()
    {
        is_config_valid = false;
        cout << "agent object instantiated" << '\n';
    }

    agent(const string &config_file)
    {
        /* Extract the agent_entity from the configuration file*/
        int result = extract_agent_entity(config_file);
        if (result == SUCCESS)
        {
            is_config_valid = true;
            cout << "Agent Creation failed" << '\n';
        }
    }

    agent(agent_entity &entity) : entity(entity)
    {
        is_config_valid = true;
    }

    int extract_agent_entity(const string &config_file)
    {
        map<string, map<string, string>> config_table;
        int result = os::is_exist(config_file);
        if (result == FILE_NOT_EXIST)
            return FILE_NOT_EXIST;
        result = config_service.read_ini_config_file(config_file, config_table);
        config_service.create_agent_entity(config_table, entity);
        return result;
    }

    int init()
    {
        log_entity syslog_entity = entity.getSysLogEntity();
        log_entity applog_entity = entity.getAppLogEntity();
        analysis_entity anlaysis_ent = entity.getAnalysisEntity();
        process_entity process_ent = entity.getProcessEntity();
        ids_entity ids_ent = entity.getIdsEntity();

        if (!is_config_valid)
            return FAILED;

        if (!loaded_modules.empty())
        {
            for (base_api *api : loaded_modules)
            {
                delete api;
            }
            loaded_modules.clear();
        }

        if (!syslog_entity.isEmpty())
        {
            base_api *syslog_api = new log_api(syslog_entity);
            loaded_modules.emplace_back(syslog_api);
        }

        if (!applog_entity.isEmpty())
        {
            base_api *applog_api = new log_api(applog_entity);
            loaded_modules.emplace_back(applog_api);
        }

        if (!anlaysis_ent.isEmpty())
        {
            base_api *analysis = new analysis_api(anlaysis_ent);
            loaded_modules.emplace_back(analysis);
        }

        if (!process_ent.isEmpty())
        {
            base_api *process = new process_api(process_ent);
            loaded_modules.emplace_back(process);
        }

        if (!ids_ent.isEmpty())
        {
            base_api *ids = new ids_api(ids_ent);
            loaded_modules.emplace_back(ids);
        }
    }

    int update()
    {
        return SUCCESS;
    }

    ~agent()
    {
        if (!loaded_modules.empty())
        {
            for (base_api *api : loaded_modules)
            {
                delete api;
            }
            loaded_modules.clear();
        }
    }

private:
    bool is_config_valid;
    agent_entity entity;
    Config config_service;
    vector<base_api *> loaded_modules;
};