#ifndef CONFIGSERVICE_HPP
#define CONFIGSERVICE_HPP
#pragma once
#define R_ID "id"
#define R_CHILD_ID "child_id"
#define R_LEVEL "level"
#define R_MAX_SIZE "maxsize"
#define R_ALERT "alert"
#define R_FREQUENCY "frequency"
#define R_TIMEFRAME "timeframe"
#define R_NAME "name"
#define R_DESCRIPTION "description"
#define R_GROUP "group"
#define R_REGEX "regex"
#define R_INFO "info"
#define R_SRC_IP "src_ip"
#define R_SRC_PORT "src_port"
#define R_DST_IP "dst_ip"
#define R_DST_PORT "dst_port"
#define R_OPTIONS "options"

#include "agentUtils.hpp"


class IniConfig
{
private:

public:
    IniConfig() = default;

    int isDigit(string number)
    {
        int digit;
        try
        {
            digit = stoi(number);
        }catch(exception &e)
        {
            digit = -1;
            cerr << e.what() << endl;
        }
        return digit;
    }

    int parseToAConfig(string rule, AConfig &config)
    {
        int returnVal = SUCCESS;
        if (rule.empty()) { AgentUtils::writeLog("Empty Rule received (AConfig)", WARNING); return WARNING; }
        const char splitter = ':';
        const char delimeter = ',';
        string attribute;
        std::stringstream ss(rule);
        while (std::getline(ss, attribute, delimeter))
        {
            size_t m = attribute.find(splitter);
            if (m == string::npos) {AgentUtils::writeLog("Invalid Rule Format"); break;}
            string key = trim(attribute.substr(0, m));
            string value = trim(attribute.substr(m+1));
            
            if (strcmp(key.c_str(), R_CHILD_ID) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid child rule id set expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.child_id = digit;
            }
            if (strcmp(key.c_str(), R_MAX_SIZE) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid log size set expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.max_log_size = digit;
            }       
            else if (strcmp(key.c_str(), R_ALERT) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid alert value set expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.alert = digit;
            }
            else if (strcmp(key.c_str(), R_LEVEL) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid level value set expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.level = digit;
            }
            else if (strcmp(key.c_str(), R_NAME) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("No Log name specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.name = value;
            }
            else if (strcmp(key.c_str(), R_FREQUENCY) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid frequency value specified, expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.frequency = digit;
            }
            else if (strcmp(key.c_str(), R_TIMEFRAME) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid TimeFrame value specified, expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.timeframe = digit;
            }
            else if (strcmp(key.c_str(), R_REGEX) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("Regex pattern not specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.regex = value;
            }
            else if (strcmp(key.c_str(), R_SRC_IP) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("src_ip pattern not specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.src_ip = value;
            }
            else if (strcmp(key.c_str(), R_SRC_PORT) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid src_port value specified, expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.src_port = digit;
            }
            else if (strcmp(key.c_str(), R_DST_IP) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("dst_ip pattern not specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.dst_ip = value;
            }
            else if (strcmp(key.c_str(), R_DST_PORT) == 0)
            {
                int digit = isDigit(value);
                if (digit < 0)
                {
                    AgentUtils::writeLog("Invalid dst_port value specofied, expecting integer", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.dst_port = digit;
            }
            else if (strcmp(key.c_str(), R_OPTIONS) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("No options specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.options = value;
            }
            else if (strcmp(key.c_str(), R_DESCRIPTION) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("No description specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.description = value;
            }
            else if (strcmp(key.c_str(), R_GROUP) == 0)
            {
                if (value.empty())
                {
                    AgentUtils::writeLog("No group specified", FAILED);
                    returnVal = FAILED;
                    break;
                }
                config.group = value;
            }
            else
            {
                AgentUtils::writeLog("Unknown rule attribute found : <" + value + ">", FAILED);
                returnVal = FAILED;
                break;
            }
        }
        return returnVal;
    }

    int cleanFile(const string filePath)
    {
        std::ofstream file(filePath, std::ios::trunc);
        if (file.is_open())
        {
            AgentUtils::writeLog(CLEAN_FILE + " <" + filePath + ">", SUCCESS);
            file.close();
            return SUCCESS;
        }
        return FAILED;
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

    int readRuleConfig(const string path, map<string, map<int, AConfig>> &table)
    {
        AConfig config;
        int result = SUCCESS;
        fstream file(path, std::ios::in | std::ios::binary);
        string line, currentSection;
        int index = 1;

        if (!file)
        {
            AgentUtils::writeLog(FILE_ERROR + path, FAILED);
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
                AgentUtils::writeLog(INVALID_FILE + path, FAILED);
                AgentUtils::writeLog("Invalid format at line " + std::to_string(index) + " : Missing closing square bracket", INFO);
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
                        AgentUtils::writeLog(INVALID_FILE + path, FAILED);
                        AgentUtils::writeLog("Invalid format at line " + std::to_string(index) + " : Found ';' in key", DEBUG);
                        result = FAILED;
                        break;
                    }
                    int digit;
                    AConfig config;
                    digit = isDigit(key);
                    if (digit < 0)
                    {
                        AgentUtils::writeLog("Invalid rule id specified" + key + " : expecting number", FAILED);
                        result = FAILED;
                        break;
                    }
                    config.id = digit;
                    
                    string value = trim(line.substr(delimiter + 1));
                    validateText(value);
                    
                    if (parseToAConfig(value, config) == FAILED)
                    {
                        AgentUtils::writeLog(INVALID_FILE + path, FAILED);
                        AgentUtils::writeLog("Invalid Config file : line number " + std::to_string(index), FAILED);
                        result = FAILED;
                        break;
                    }
                    if (currentSection.empty())
                    {
                        AgentUtils::writeLog("No Log name specified", FAILED);
                        result = FAILED;
                        break;
                    }
                    config.name = currentSection;
                    table[currentSection][config.id] = config;
                }
            }
            index++;
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
            AgentUtils::writeLog(INVALID_FILE + path, FAILED);
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
                AgentUtils::writeLog(INVALID_FILE + path, FAILED);
                AgentUtils::writeLog("Invalid Config file : line number " + std::to_string(index), FAILED);
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