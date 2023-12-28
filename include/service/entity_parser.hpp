#ifndef ENTITY_PARSER
#define ENTITY_PARSER
#pragma once 

#include "entity.hpp"
#include "service/config_service.hpp"

class entity_parser
{
private:
    Config config;

public:
    log_entity get_log_entity(map<string, map<string, string>> &config_table, const string &name)
    {
        log_entity entity;
        entity.format = name;
        entity.read_path =  config_table[name]["read_path"];
        entity.write_path =  config_table[name]["write_path"];
        entity.time_pattern =  config_table[name]["time_pattern"];
        entity.storage_type =  config_table[name]["storage_type"];
        string json_attributes =  config_table[name]["json_attributes"];
        if (!json_attributes.empty())
        {
            entity.json_attributes = config.to_vector(json_attributes, ',');
        }
        else
        {
            entity.json_attributes.clear();
        }

        string log_levels =  config_table[name]["log_levels"];
        if (!log_levels.empty())
        {
            entity.log_levels = config.to_vector(log_levels, ',');
        }
        else
        {
            entity.log_levels.clear();
        }
        string delimeter =  config_table[name]["delimeter"];
        if (delimeter.empty())
        {
            delimeter = " ";
        }
        entity.delimeter = delimeter[0];
        string remote =  config_table[name]["remote"];
        if (remote.empty())
        {
            remote = "N";
        }
        entity.remote = remote[0];

        return entity;
    }
};

#endif