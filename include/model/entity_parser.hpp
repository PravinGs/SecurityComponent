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

        entity.read_path = config_table[name]["read_path"];
        entity.write_path = config_table[name]["write_path"];
        entity.time_pattern = config_table[name]["time_pattern"];
        entity.storage_type = config_table[name]["storage_type"];
        string format = config_table[name]["format"];
        if (format.empty())
        {
            format = name;
        }
        entity.format = format;
        string json_attributes = config_table[name]["json_attributes"];
        if (!json_attributes.empty())
        {
            entity.json_attributes = config.to_vector(json_attributes, ',');
        }
        else
        {
            entity.json_attributes.clear();
        }

        string log_levels = config_table[name]["log_levels"];
        if (!log_levels.empty())
        {
            entity.log_levels = config.to_vector(log_levels, ',');
        }
        else
        {
            entity.log_levels.clear();
        }
        string delimeter = config_table[name]["delimeter"];
        if (delimeter.empty())
        {
            delimeter = " ";
        }
        entity.delimeter = delimeter[0];
        string remote = config_table[name]["remote"];
        if (remote.empty())
        {
            remote = "N";
        }
        entity.remote = remote[0];

        return entity;
    }

    analysis_entity get_analysis_entity(map<string, map<string, string>> &config_table)
    {
        analysis_entity entity;
        entity.log_path = config_table["analysis"]["log_path"];
        entity.decoder_path = config_table["analysis"]["decoder_path"];
        entity.rules_path = config_table["analysis"]["rules_path"];
        entity.write_path = config_table["analysis"]["write_path"];
        entity.time_pattern = config_table["analysis"]["time_pattern"];
        entity.storage_type = config_table["analysis"]["storage_type"];
        return entity;
    }

    process_entity get_process_entity(map<string, map<string, string>> &config_table)
    {
        process_entity entity;
        entity.write_path = config_table["process"]["write_path"];
        entity.time_pattern = config_table["process"]["time_pattern"];
        entity.storage_type = config_table["process"]["storage_type"];
        return entity;
    }

    patch_entity get_patch_entity()
    {
        patch_entity entity;
        entity.application = config_table["firmware"]["application"];
        entity.application_root_path = config_table["firmware"]["application_root_path"];
        try
        {
            entity.max_download_speed = std::stoi(config_table["firmware"]["max_download_speed"]);
            entity.min_download_speed = std::stoi(config_table["firmware"]["min_download_speed"]);
            entity.retry_time_out = std::stoi(config_table["firmware"]["retry_time_out"]);
        }
        catch (const std::exception &e)
        {
            string error = e.what();
            agent_utils::write_log("patch_entitty get_patch_entity() exception occured: " + error, FAILED);
        }

        return entity;
    }
};

#endif