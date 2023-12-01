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

    agent(std::shared_ptr<agent_entity> newEntity) : entity(newEntity), is_config_valid(true)
    {
    }

    agent(agent_entity *newEntity) : entity(newEntity), is_config_valid(true)
    {
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
        log_entity syslog_entity = entity->getSysLogEntity();
        log_entity applog_entity = entity->getAppLogEntity();
        analysis_entity anlaysis_ent = entity->getAnalysisEntity();
        process_entity process_ent = entity->getProcessEntity();
        ids_entity ids_ent = entity->getIdsEntity();

        if (!is_config_valid)
            return FAILED;

        if (!loaded_modules.empty())
        {
            loaded_modules.clear(); // I should use erase method to remove specific modules
        }

        if (!syslog_entity.isEmpty())
        {
            std::shared_ptr<base_api> syslog_api = std::make_shared<log_api>(syslog_entity);
            loaded_modules.emplace_back(syslog_api);
        }

        if (!applog_entity.isEmpty())
        {
            std::shared_ptr<base_api>  applog_api = std::make_shared<log_api>(applog_entity);
            loaded_modules.emplace_back(applog_api);
        }

        if (!anlaysis_ent.isEmpty())
        {
            std::shared_ptr<base_api>  analysis = std::make_shared<analysis_api>(anlaysis_ent);
            loaded_modules.emplace_back(analysis);
        }

        if (!process_ent.isEmpty())
        {
            std::shared_ptr<base_api>  process = std::make_shared<process_api>(process_ent);
            loaded_modules.emplace_back(process);
        }

        if (!ids_ent.isEmpty())
        {
            std::shared_ptr<base_api>  ids = std::make_shared<ids_api>(ids_ent);
            loaded_modules.emplace_back(ids);
        }
    }

    int update()
    {
        return SUCCESS;
    }

private:
    std::shared_ptr<agent_entity> entity;
    bool is_config_valid;
    Config config_service;
    vector<std::shared_ptr<base_api>> loaded_modules;
};