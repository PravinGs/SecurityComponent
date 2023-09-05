#ifndef CONFIGSERVICE_HPP
#define CONFIGSERVICE_HPP
#pragma once

#include "utils/agentUtils.hpp"

class IniConfig
{
private:
    pugi::xml_document doc;

public:
    IniConfig() = default;

    AConfig getAConfigById(const int ruleId)
    {
        AConfig config;
        return config;
    }

    int isDigit(string number)
    {
        if (number.empty()) return -1;
        int digit;
        try
        {
            digit = stoi(number);
        }
        catch (exception &e)
        {
            digit = -1;
            cerr << e.what() << endl;
        }
        return digit;
    }

    int parseToAConfig(string fileName, map<string, map<int, AConfig>> &table)
    {
        pugi::xml_parse_result result = doc.load_file(fileName.c_str());

        if (!result)
        {
            string error = result.description();
            cout << "Error: " << error << endl;
            AgentUtils::writeLog(FILE_ERROR + fileName, FAILED);
            return FAILED;
        }

        pugi::xml_node root = doc.child("group");

        for (pugi::xml_node groupNode = root; groupNode; groupNode = groupNode.next_sibling("group"))
        {
            AConfig rule;

            std::string currentSection = root.attribute("name").value();

            if (!currentSection.empty())
            {
                rule.group = currentSection;
            }
            for (pugi::xml_node ruleNode = groupNode.child("rule"); ruleNode; ruleNode = ruleNode.next_sibling("rule"))
            {
                int digit;
                string str;
                digit = isDigit(ruleNode.attribute("id").value());
                if (digit != -1)
                {
                    rule.id = digit;
                }
                digit = -1;

                digit = isDigit(ruleNode.attribute("level").value());
                if (digit != -1)
                {
                    rule.level = digit;
                }
                digit = -1;
                digit = isDigit(ruleNode.attribute("frequency").value());
                if (digit != -1)
                {
                    rule.frequency = digit;
                }
                digit = -1;
                digit = isDigit(ruleNode.attribute("timeframe").value());
                if (digit != -1)
                {
                    rule.timeframe = digit;
                }
                digit = -1;
                digit = isDigit(ruleNode.attribute("ignore").value());
                if (digit != -1)
                {
                    rule.ignore = digit;
                }
                digit = -1;
                digit = isDigit(ruleNode.child_value("if_sid"));
                if (digit != -1)
                {
                    rule.if_sid = digit;
                }
                digit = -1;
                digit = isDigit(ruleNode.child_value("same_source_ip"));
                if (digit != -1){
                    rule.same_source_ip = 1;
                }
                digit = -1;
                str = ruleNode.child_value("status_pcre2");
                if (!str.empty())
                {
                    rule.status_pcre2 = str;
                }

                str = ruleNode.child_value("id_pcre2");
                if (!str.empty())
                {
                    rule.id_pcre2 = str;
                }

                str = ruleNode.child_value("extra_data_pcre2");
                if (!str.empty())
                {
                    rule.extra_data_pcre2 = str;
                }

                str = ruleNode.child_value("pcre2");
                if (!str.empty())
                {
                    rule.pcre2 = str;
                }

                str = ruleNode.child_value("program_name_pcre2");
                if (!str.empty())
                {
                    rule.program_name_pcre2 = str;
                }

                str = ruleNode.child_value("scrip"); /*Not sure */
                if (!str.empty())
                {
                    rule.script = str;
                }

                str = ruleNode.child_value("group");
                if (!str.empty())
                {
                    rule.group = str;
                }

                str = ruleNode.child_value("description");
                if (!str.empty())
                {
                    rule.description = str;
                }

                str = ruleNode.child_value("decoded_as");
                if (!str.empty())
                {
                    rule.decoded_as = str;
                }

                str = ruleNode.child_value("category"); /*check*/
                if (!str.empty())
                {
                    rule.categories = str;
                }

                str = ruleNode.child_value("action");
                if (!str.empty())
                {
                    rule.action = str;
                }

                str = ruleNode.child_value("options");
                if (!str.empty())
                {
                    rule.options = str;
                }

                digit = isDigit(ruleNode.child_value("if_matched_sid"));
                if (digit != -1)
                {
                    rule.if_matched_id = digit;
                }
                digit = -1;
                str = ruleNode.child_value("url_pcre2");
                if (!str.empty())
                {
                    rule.url_pcre2 = str;
                }

                str = ruleNode.child_value("compiled_rule");
                if (!str.empty())
                {
                    rule.compiled_rule = str;
                }

                str = ruleNode.child_value("hostname_pcre2");
                if (!str.empty())
                {

                    rule.hostname_pcre2 = str;
                }
                table[currentSection][rule.id] = rule;
            }
        }
        AgentUtils::writeLog("XML parsing success for " + fileName, DEBUG);
        return SUCCESS;
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
        AgentUtils::writeLog("Reading " + path, DEBUG);
        int result = SUCCESS;
        int isFile = 0;
        if (OS::isDirExist(path) && std::filesystem::is_regular_file(path))
        {
            isFile = 1;
        }

        if (isFile)
        {
            result = parseToAConfig(path, table);
        }
        else
        {
            string currentFile = path;
            vector<string> files;
            if (currentFile[currentFile.size() - 1] == '/')
            {
                currentFile = currentFile.substr(0, currentFile.find_last_of('/'));
            }
            result = OS::getRegularFiles(currentFile, files);

            if (result == FAILED)
                return FAILED;

            if (files.size() == 0)
            {
                AgentUtils::writeLog(INVALID_PATH + path, FAILED);
                return FAILED;
            }
            for (string file : files)
            {
                result = parseToAConfig(file, table);
            }
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