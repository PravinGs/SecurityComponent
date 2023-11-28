#pragma once 
#include "common.hpp"
#include "base_api.hpp"
#include "service/config_service.hpp"
#include "entity/agent_entity.hpp"


class agent
{
public:

    agent()
    {
        cout << "agent object instantiated" << '\n';
    }

    agent(const string& config_file)
    {   
        /* Extract the agent_entity from the configuration file*/
       int result = extract_agent_entity(config_file);
       if (result != SUCCESS)
       {
        is_config_valid = false;
        cout << "Agent Creation failed" << '\n';
       }
       
    }

    agent(agent_entity& entity):entity(entity)
    {
        /*Initialize this object */
    }

    int extract_agent_entity(const string& config_file)
    {
        map<string, map<string, string>> config_table;
        int result = os::is_exist(config_file);
        if (result == FILE_NOT_EXIST) return FILE_NOT_EXIST;
        result = config_service.read_ini_config_file(config_file, config_table);
        config_service.create_agent_entity(config_table, entity);
        return result;
    }

    int init(const agent_entity& entity)
    {
        /*Validate entity*/
        if (!is_config_valid)
        {
            return FAILED;
        }
        // To be continued
        return SUCCESS;
    }

    int update()
    {
        return SUCCESS;
    }

private:
    bool is_config_valid = true;
    agent_entity entity;
    Config config_service;
    vector<base_api*> loaded_modules;
};