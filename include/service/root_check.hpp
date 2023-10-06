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

class RootCheck
{
    private:
        ICommand *command;
        TrojenCheck trojanCheck;
        SysCheck sysCheck;
        ProcessCheck processCheck;
        PortCheck portCheck;
        DevCheck devCheck;
        InterfaceCheck interfaceCheck;
        string trojanSourceFile;
        string sysSourceFile;
        vector<string> reportCommands = {
                                            "ApparmorStatus:aa-status",
                                            "PortDetails:netstat -tuln",
                                            "NetworkConnection:lsof -i -P",
                                            "LoginActivity:last -n 5",
        };

    private:
        void executeCommands(Json::Value & json)
        {
            for (const string& s: reportCommands)
            {
                vector<string> output;
                size_t mid = s.find_first_of(':');
                command->readCommand(s.substr(mid+1), output);
                string name = s.substr(0, mid);
                json[name] = Json::Value(Json::arrayValue);
                for (string line: output){
                    json[name].append(line);
                }
            }
        }

    public:
        RootCheck()
        {
            command = new Command();
            this->trojanSourceFile = TROJAN_SOURCE_FILE;
            this->sysSourceFile    = SYS_SOURCE_FILE;
        }

        RootCheck(string trojanSourceFile, string sysSourceFile)
        {
            this->trojanSourceFile = trojanSourceFile;
            this->sysSourceFile    = sysSourceFile; 
        }

        

        int start()
        {
            AgentUtils::writeLog("Starting trojan root check.", DEBUG);
            Json::Value rootCheck;
            vector<string> reports;
            Json::StreamWriterBuilder writerBuilder;
            if (trojanCheck.check(trojanSourceFile, reports))
            {
                rootCheck["TrojanCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    size_t mid = report.find_first_of(',');
                    json["location"] = report.substr(0, mid);
                    json["pattern"]  = report.substr(mid + 1);
                    rootCheck["TrojanCheck"].append(json);
                }
                reports.clear();
                AgentUtils::writeLog("Trojan root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Trojan root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed trojan root check.", DEBUG);
            AgentUtils::writeLog("Starting sysfiles root check.");
            if (sysCheck.check(sysSourceFile, reports))
            {
                rootCheck["SysFileCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    size_t mid = report.find_first_of(',');
                    json["name"]  = report.substr(mid + 1);
                    json["location"] = report.substr(0, mid);
                    rootCheck["SysFileCheck"].append(json);
                }
                reports.clear();
                AgentUtils::writeLog("SysFiles root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Sysfiles root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed sysfiles root check.", DEBUG);
            AgentUtils::writeLog("Starting network interface root check.", DEBUG);
            if (interfaceCheck.check(reports))
            {
                rootCheck["PromiscuousCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    json["interface"] = report;
                    rootCheck["PromiscuousCheck"].append(json);
                }
                reports.clear();
                AgentUtils::writeLog("network interface  root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("network interface root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed network interface root Check.", DEBUG);
            AgentUtils::writeLog("Starting dev root check.", DEBUG);
            if (devCheck.check(reports))
            {
                rootCheck["HiddenFileCheck"] = Json::Value(Json::arrayValue);
                for (string report: reports)
                {
                    Json::Value json;
                    json["path"] = report;
                    rootCheck["HiddenFileCheck"].append(json);
                }
                reports.clear();
                AgentUtils::writeLog("dev root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("dev root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed dev root check.", DEBUG);
            executeCommands(rootCheck);
            std::fstream file("/home/champ/SecurityComponent/syscheckreport.json", std::ios::out);
            std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
            writer->write(rootCheck, &file);
            file.close();
            /*AgentUtils::writeLog("Starting process root check", DEBUG);
            AgentUtils::writeLog("This process might take more time", DEBUG);
            if (processCheck.check())
            {
                AgentUtils::writeLog("Process root check completed successfully.", SUCCESS);
            }
            else
            {
                 AgentUtils::writeLog("Process root check failed", FAILED);
            }
            */
            AgentUtils::writeLog("Completed process root check", INFO);
            return SUCCESS;
        }

        ~RootCheck(){
            delete command;
        }

};

#endif