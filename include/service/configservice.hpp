#ifndef CONFIGSERVICE_HPP
#define CONFIGSERVICE_HPP
#pragma once

#include "agentUtils.hpp"

class IniConfig
{
public:
    IniConfig() = default;

    void cleanFile(const string filePath)
    {
        std::ofstream file(filePath, std::ios::trunc);
        if (file.is_open())
        {
            AgentUtils::writeLog("Cleaning the json " + filePath, SUCCESS);
            file.close();
        }
        return;
    }

    string trim(string line)
    {
        const auto strBegin = line.find_first_not_of(" \t");
        if (strBegin == std::string::npos)
            return "";

        const auto strEnd = line.find_last_not_of(" \t");
        const auto strRange = strEnd - strBegin + 1;

        string str = line.substr(strBegin, strRange);
        return (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') ? str.substr(1, str.length() - 2) : str;
    }

    vector<string> toVector(string columns, const char sep)
    {
        vector<string> names;
        std::stringstream iss(columns);
        string name;
        while (std::getline(iss, name, sep))
        {
            names.push_back(trim(name));
        }
        return names;
    }

    bool validateText(string &line)
    {
        size_t delimiter = line.find(';');
        bool result = true;
        if (delimiter != string::npos)
        {
            string content = trim(line.substr(0, delimiter));
            line = content;
            result = false;
        }
        return result;
    }

    int readConfigFile(const string path, map<string, map<string, string>> &table)
    {
        int result = SUCCESS;
        fstream file(path, std::ios::in | std::ios::binary);
        string line, currentSection;
        int index = 1;

        if (!file)
        {
            AgentUtils::writeLog("Failed to open " + path + " check file path and file permission", FAILED);
            return FAILED;
        }

        while (std::getline(file, line))
        {
            line = trim(line);

            if (line.empty() || line[0] == ';')
            {
                continue;
            }
            else if (line[0] == '[' && line[line.size() - 1] == ']')
            {
                currentSection = trim(line.substr(1, line.size() - 2));
            }
            else if (line[0] == '[' && line[line.size() - 1] != ']')
            {
                AgentUtils::writeLog("Invalid Config file : " + path + " [ line number " + std::to_string(index) + " ]", FAILED);
                result = FAILED;
                break;
            }
            else
            {
                size_t delimiter = line.find('=');
                if (delimiter != string::npos)
                {
                    string key = trim(line.substr(0, delimiter));
                    if (!validateText(key))
                    {
                        AgentUtils::writeLog("Invalid Config file : line number " + std::to_string(index), FAILED);
                        result = FAILED;
                        break;
                    }
                    string value = trim(line.substr(delimiter + 1));
                    validateText(value);
                    table[currentSection][key] = value;
                }
            }
            index++;
        }
        return result;
    }

    ~IniConfig() {}
};

#endif