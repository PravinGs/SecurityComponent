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

    patch_entity get_patch_entity(map<string, map<string, string>> &config_table)
    {
        patch_entity entity;
        entity.application = config_table["firmware"]["application"];
        entity.application_root_path = config_table["firmware"]["application_root_path"];
        entity.username = config_table["firmware"]["username"];
        entity.password = config_table["firmware"]["password"];
        entity.url      = config_table["firmware"]["url"];
        entity.ca_cert_path = config_table["firmware"]["ca_cert_path"];
        entity.client_cert_path = config_table["firmware"]["client_cert_path"];
        try
        {
            entity.max_download_speed = std::stoi(config_table["firmware"]["max_download_speed"]);
            entity.min_download_speed = std::stoi(config_table["firmware"]["min_download_speed"]);
            entity.retry_time_out = std::stoi(config_table["firmware"]["retry_time_out"]);
            entity.retry_count  = std::stoi(config_table["firmware"]["retry_count"]);
        }
        catch (const std::exception &e)
        {
            string error = e.what();
            agent_utils::write_log("patch_entitty get_patch_entity() exception occured: " + error, FAILED);
        }

        return entity;
    }

    mqtt_entity get_mqtt_entity(map<string, map<string, string>> &config_table)
    {
        mqtt_entity entity;
        entity.client_id = config_table["mqtt"]["client_id"];
        entity.conn_string = config_table["mqtt"]["address"];
        entity.topics = config.to_vector(config_table["mqtt"]["topics"], ',') ;
        entity.ca_cert_path = config_table["mqtt"]["ca_cert_path"];
        entity.client_cert_path = config_table["mqtt"]["client_cert_path"];
        try
        {
            entity.port = std::stoi(config_table["mqtt"]["port"]);
        }
        catch(const std::exception& e)
        {
            entity.port = 8000;
            std::cerr << e.what() << '\n';
            agent_utils::write_log("Mqtt port number not valid", FAILED);
        }
        
        return entity;
    }

    rest_entity get_rest_entity(map<string, map<string, string>> &config_table)
    {
        rest_entity entity;
        entity.logs_post_url = config_table["cloud"]["logs_post_url"];
        entity.ids_post_url = config_table["cloud"]["ids_post_url"];
        entity.resources_post_url = config_table["cloud"]["resources_post_url"];
        entity.ca_cert_path = config_table["cloud"]["ca_cert_path"];
        entity.client_cert_path = config_table["cloud"]["client_cert_path"];
        return entity;
    }

    conn_entity get_conn_entity(map<string, map<string, string>> &config_table, const string& type)
    {
        conn_entity entity;
        string cert_pem = type + "_cert_path";
        string key_pem = type + "_private_key_path";
        string port;
        entity.ca_pem = config_table["connection"]["ca_cert_path"];
        entity.cert_pem = config_table["connection"][cert_pem];
        entity.key_pem = config_table["connection"][key_pem];
        port = config_table["connection"]["port"];
        try
        {
            entity.port = std::stoi(port);
        }
        catch(const std::exception& e)
        {
            string error = e.what();
            agent_utils::write_log("entity_parser: get_conn_entity: " + error, FAILED);
        }
        
        if (type == "client")
        {
            entity.conn_string = config_table["connection"]["host_address"] + ":" + port;
        }
        return entity;
    }

};

#endif