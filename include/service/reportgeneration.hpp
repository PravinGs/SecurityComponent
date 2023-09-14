#ifndef REPORT_GEN_HPP
#define REPORT_GEN_HPP
#pragma once
#include "agentUtils.hpp"

class Report
{
    public:
        int generateFromFile(string path);
        int generate(vector<LOG_EVENT> alerts);
};

int Report::generateFromFile(string path) 
{
    Json::Value json;
    json["Alerts"] = Json::Value(Json::arrayValue);
    Json::Value jsonValue;
    Json::StreamWriterBuilder writerBuilder;
    string sep = "|";
    string line;
    fstream file(path, std::ios::in); 
    if(!file)
    {
        cout << "File path not valid " << path << "\n";
        return FAILED;
    }

    while(std::getline(file,line))
    {
        string timestamp;
        string user;
        string program;
        string log;
        string ruleId;
        AConfig config;
        std::stringstream ss(line);
        std::getline(ss, timestamp, '|');
        std::getline(ss, user, '|');
        std::getline(ss, program, '|');
        std::getline(ss, log, '|');
        std::getline(ss, ruleId, '|');

        jsonValue["timestamp"] = timestamp;
        jsonValue["user"] = user;
        jsonValue["program"] = program;
        jsonValue["log"] = log;
        jsonValue["ruleId"] = ruleId;
        /*Read stoi()
        getAConfigById(ruleId) 
        Aconfig and add decoded_as, description, level, group,*/
        json["Alerts"].append(jsonValue);
    }
    
    file.close();
    
    std::ofstream ofile(path+"-report.json"); /*need update*/
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(json, &ofile);
    ofile.close();
    AgentUtils::writeLog("Log written to " + path, SUCCESS); 
    return SUCCESS;
}

#endif