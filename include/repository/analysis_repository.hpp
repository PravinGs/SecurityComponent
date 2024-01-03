#ifndef ANALYSIS_REPOSITORY
#define ANALYSIS_REPOSITORY

#pragma once

#include "model/analysis_model.hpp"
#include "service/config_service.hpp"

class analysis_repository
{

private:
    Config config;
    std::unordered_map<string, std::unordered_map<int, aconfig>> rules;
    std::unordered_map<string, decoder> decoder_list;
public:

    std::unordered_map<string, decoder> get_decoder_list()
    {
        return decoder_list;
    }

    std::unordered_map<string, std::unordered_map<int, aconfig>> get_aconfig_rules()
    {
        return rules;
    }

    bool read_config_files(const string &decoder_path, const string &rule_path)
    {
        int result = config.read_decoder_config(decoder_path, decoder_list);
        if (result == FAILED)
        {
            return false;
        }
        result = config.read_xml_rule_config(rule_path, rules);
        if (result == FAILED)
        {
            return false;
        }
        return true;
    }

    int save(const vector<log_event> &alerts)
    {
        string host;
        agent_utils::get_hostname(host);
        if (alerts.size() == 0)
        {
            agent_utils::write_log("analysis_repository: save: no decoder matched", INFO);
            return SUCCESS;
        }
        string filePath = os::get_json_write_path("log-analysis-report");
        Json::Value json;
        json["OrgId"] = 5268;
        json["Source"] = host;
        json["AppName"] = "system_events";
        json["Alerts"] = Json::Value(Json::arrayValue);
        Json::Value alert;
        Json::StreamWriterBuilder writer_builder;

        for (const auto &log : alerts)
        {
            aconfig config = get_rule(log.group, log.rule_id);
            if (config.id <= 0)
            {
                agent_utils::write_log("analysis_repository: save: unrecognized rule, rule_id=" + std::to_string(log.rule_id) + " RuleGroup=" + log.group, WARNING);
            }
            else
            {
                // print_log_details(config, log);
                aconfig child = config;
                // string group;
                alert["TimeStamp"] = log.timestamp;
                alert["User"] = log.user;
                alert["Program"] = log.program;
                alert["Message"] = log.message;
                alert["Category"] = log.decoded;
                alert["MatchedRule"] = config.id;
                alert["LogLevel"] = config.level;
                while (child.if_sid > 0)
                {
                    child = get_rule(log.group, child.if_sid);
                }

                // group = (config.group.empty()) ? child.group : config.group;

                alert["Section"] = log.group;
                alert["Description"] = (config.description.empty()) ? child.description : config.description;
                json["Alerts"].append(alert);
            }
        }
        std::ofstream ofile(filePath); /*need update*/
        std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
        writer->write(json, &ofile);
        ofile.close();
        agent_utils::write_log("analysis_repository: save: log written to " + filePath, SUCCESS);
        return SUCCESS;
    }

    aconfig get_rule(const string &group, const int rule_id)
    {
        aconfig config;
        for (const auto &parent : rules)
        {
            for (const auto &child : parent.second)
            {
                if (child.first == rule_id)
                {
                    config = child.second;
                    break;
                }
            }
        }
        return config;
    }

    int print_log_details(const aconfig &rule_info, const log_event &log_info)
    {
        aconfig child = rule_info;
        string group = log_info.group;
        cout << "Timestamp : " << log_info.timestamp << "\n";
        cout << "user      : " << log_info.user << "\n";
        cout << "program   : " << log_info.program << "\n";
        cout << "log       : " << log_info.message << "\n";
        cout << "category  : " << log_info.decoded << "\n";
        cout << "rule      : " << child.id << "\n";
        cout << "level     : " << child.level << "\n";
        while (child.if_sid > 0)
        {
            child = get_rule(log_info.group, child.if_sid);
        }
        if (rule_info.description.empty())
        {
            group = child.decoded_as.empty() ? log_info.group : child.decoded_as;
        }
        else
        {
            group = rule_info.decoded_as.empty() ? log_info.group : rule_info.decoded_as;
        }
        cout << "group  : " << group << "\n";
        if (rule_info.description.empty())
        {
            cout << "Description: " << child.description << "\n";
        }
        else
        {
            cout << "Description: " << rule_info.description << "\n";
        }
        return SUCCESS;
    }
    
    };

#endif