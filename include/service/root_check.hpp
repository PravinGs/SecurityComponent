#ifndef ROOT_CHECK
#define ROOT_CHECK
#pragma once

#include "rootkit/dev_check.hpp"
#include "rootkit/interface_check.hpp"
#include "rootkit/ports_check.hpp"
#include "rootkit/process_check.hpp"
#include "rootkit/trojen_check.hpp"
#include "rootkit/sysfile_check.hpp"
#include "service/command_service.hpp"

#define TROJAN_SOURCE_FILE "/etc/scl/ids/rootkit_trojans.txt"
#define SYS_SOURCE_FILE "/etc/scl/ids/rootkit_files.txt"

class root_check
{
    private:
        ICommand *c_service;
        trojan_check t_check;
        sys_check s_check;
        process_check p_check;
        port_check pt_check;
        dev_check d_check;
        interface_check if_check;
        string trojan_source_file;
        string sys_source_file;
        vector<string> report_commands = {
                                            "ApparmorStatus:aa-status",
                                            "PortDetails:netstat -tuln",
                                            "NetworkConnection:lsof -i -P",
                                            "LoginActivity:last -n 5",
        };

    private:
        void execute_commands(Json::Value & json)
        {
            for (const string& s: report_commands)
            {
                vector<string> output;
                size_t mid = s.find_first_of(':');
                c_service->read_command(s.substr(mid+1), output);
                string name = s.substr(0, mid);
                json[name] = Json::Value(Json::arrayValue);
                for (string line: output){
                    json[name].append(line);
                }
            }
        }

    public:
        root_check()
        {
            c_service = new command();
            this->trojan_source_file = TROJAN_SOURCE_FILE;
            this->sys_source_file    = SYS_SOURCE_FILE;
        }

        root_check(const string& trojan_source_file, const string& sys_source_file)
        {
            this->trojan_source_file = trojan_source_file;
            this->sys_source_file    = sys_source_file; 
        }

        int start(Json::Value & root_check)
        {
            agent_utils::write_log("root_check: start: starting trojan root check.", DEBUG);        
            vector<string> reports;
            Json::StreamWriterBuilder writer_builder;
            if (t_check.check(trojan_source_file, reports))
            {
                root_check["Trojan_check"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    size_t mid = report.find_first_of(',');
                    json["location"] = report.substr(0, mid);
                    json["pattern"]  = report.substr(mid + 1);
                    root_check["Trojan_check"].append(json);
                }
                reports.clear();
                agent_utils::write_log("root_check: start: trojan root check completed successfully.", SUCCESS);
            }
            else
            {
                agent_utils::write_log("root_check: start: trojan root check failed", FAILED);
            }
            agent_utils::write_log("root_check: start: completed trojan root check.", DEBUG);
            agent_utils::write_log("root_check: start: starting sysfiles root check.");
            if (s_check.check(sys_source_file, reports))
            {
                root_check["SysFileCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    size_t mid = report.find_first_of(',');
                    json["name"]  = report.substr(mid + 1);
                    json["location"] = report.substr(0, mid);
                    root_check["SysFileCheck"].append(json);
                }
                reports.clear();
                agent_utils::write_log("root_check: start: sysfiles root check completed successfully.", SUCCESS);
            }
            else
            {
                agent_utils::write_log("root_check: start: sysfiles root check failed", FAILED);
            }
            agent_utils::write_log("root_check: start: completed sysfiles root check.", DEBUG);
            agent_utils::write_log("root_check: start: starting network interface root check.", DEBUG);
            if (if_check.check(reports))
            {
                root_check["PromiscuousCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    json["interface"] = report;
                    root_check["PromiscuousCheck"].append(json);
                }
                reports.clear();
                agent_utils::write_log("root_check: start: network interface  root check completed successfully.", SUCCESS);
            }
            else
            {
                agent_utils::write_log("root_check: start: network interface root check failed", FAILED);
            }
            agent_utils::write_log("root_check: start: completed network interface root Check.", DEBUG);
            agent_utils::write_log("root_check: start: starting dev root check.", DEBUG);
            if (d_check.check(reports))
            {
                root_check["HiddenFileCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    json["path"] = report;
                    root_check["HiddenFileCheck"].append(json);
                }
                reports.clear();
                agent_utils::write_log("root_check: start: dev root check completed successfully.", SUCCESS);
            }
            else
            {
                agent_utils::write_log("root_check: start: dev root check failed", FAILED);
            }
            agent_utils::write_log("root_check: start: completed dev root check.", DEBUG);
            execute_commands(root_check);
            std::fstream file("/home/champ/SecurityComponent/sys_checkreport.json", std::ios::out);
            std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
            writer->write(root_check, &file);
            file.close();
            agent_utils::write_log("root_check: start: completed process root check", INFO);
            return SUCCESS;
        }

        ~root_check(){
            delete c_service;
        }

};

#endif