#ifndef CONFIGSERVICE_HPP
#define CONFIGSERVICE_HPP
#pragma once

#include "agentUtils.hpp"

/**
 * @brief Configuration Management Class
 *
 * The `Config` class is a versatile configuration management class that handles a wide range of configuration file
 * formats. It provides functionality for reading and managing configuration settings from various types of configuration
 * files, allowing the application to adapt to different configuration file formats and customize its behavior.
 *
 * This class serves as a central hub for configuration-related activities, making it flexible and adaptable to different
 * configuration needs.
 */
class Config
{
private:
    /**
     * @brief XML Document
     *
     * The `doc` member variable is an instance of the `pugi::xml_document` class used to represent an XML document.
     * It is a private member of the class and serves as the container for parsing and manipulating XML data.
     */
    pugi::xml_document doc;

public:
    /**
     * @brief Default Constructor for Config
     *
     * The default constructor for the `Config` class creates an instance of the class with default settings. It initializes
     * the object to its default state.
     */
    Config() = default;

    AConfig getAConfigById(const int ruleId)
    {
        AConfig config;
        return config;
    }

    /**
     * @brief Convert String to Integer with Error Handling
     *
     * The `isDigit` function is used to convert a string representing a number into an integer, while also providing error
     * handling for cases where the conversion fails due to invalid input.
     *
     * @param[in] number The input string to be converted to an integer.
     * @return An integer result code:
     *         - The converted integer value if the conversion was successful.
     *         - -1 if the input string is not a valid integer or conversion fails.
     */
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
            cerr << e.what() << "\n";
        }
        return digit;
    }

    int parseToDecoder(const string & fileName, std::unordered_map<string, decoder>& table)
    {   
        /*
            for (const auto& d: decoders)
            {
                decoder s = d.second;
                cout << "<decoder name=" << d.first << " </decoder>\n";
                cout << "    <parent> " << s.parent << " </parent>\n";
                cout << "    <program_name_pcre2> " << s.program_name_pcre2 << " </program_name_pcre2>\n";
                cout << "    <pcre2> " << s.pcre2 << " </pcre2>\n";
                cout << "    <prematch_pcre2> " << s.prematch_pcre2 << " </prematch_pcre2>\n";
                cout << "    <order> " << s.order << " </order>\n";
            }
        */     
        pugi::xml_parse_result result = doc.load_file(fileName.c_str());

        if (!result)
        {
            string error = result.description();
            AgentUtils::writeLog(FILE_ERROR + fileName, FAILED);
            return FAILED;
        }
        pugi::xml_node root = doc.child("root");
        for (pugi::xml_node groupNode = root.child("decoder"); groupNode; groupNode = groupNode.next_sibling("decoder"))
        {
            decoder d;
            std::string currentSection = groupNode.attribute("name").value();
            if (!currentSection.empty())
            {
                d.decoder=currentSection;
            }
            currentSection = groupNode.child_value("parent");
            if (!currentSection.empty())
            {
                d.parent=currentSection;
            }
            currentSection = groupNode.child_value("program_name_pcre2");
            if (!currentSection.empty())
            {
                d.program_name_pcre2=currentSection;
            }
            currentSection = groupNode.child_value("pcre2");
            if (!currentSection.empty())
            {
                d.pcre2=currentSection;
            }
            currentSection = groupNode.child_value("order");
            if (!currentSection.empty())
            {
                d.order=currentSection;
            }
            currentSection = groupNode.child_value("prematch_pcre2");
            if (!currentSection.empty())
            {
                d.prematch_pcre2=currentSection;
            }
            currentSection = groupNode.child_value("fts");
            if (!currentSection.empty())
            {
                d.fts=currentSection;
            }
            currentSection = groupNode.child_value("offset");
            if (!currentSection.empty())
            {
                d.offset=currentSection;
            }
            table[d.decoder] = d;
        }
        AgentUtils::writeLog("XML parsing success for " + fileName, DEBUG);
        return SUCCESS;
    }

    void extractRuleAttributes(pugi::xml_node & root, std::unordered_map<string, std::unordered_map<int, AConfig>> &table)
    {
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
    }

    /**
     * @brief Parse XML Nodes into AConfig Table
     *
     * The `parseToAConfig` function is used to parse XML nodes from a specified XML file and populate a structured table of
     * `AConfig` objects. Each XML node represents configuration data, and the parsed data is organized within the table.
     *
     * @param[in] fileName The file name of the XML file containing configuration nodes.
     * @param[out] table A reference to a map for storing the parsed configuration data. The map is structured as follows:
     *                  - The keys represent unique identifiers for each configuration.
     *                  - The values are maps containing configuration settings organized by integer keys.
     * @return An integer result code:
     *         - SUCCESS: The XML nodes were successfully parsed and populated into the AConfig table.
     *         - FAILED: The operation encountered errors and failed to parse or populate the configuration data.
     */
    int parseToAConfig(string fileName, std::unordered_map<string, std::unordered_map<int, AConfig>> &table)
    {
        pugi::xml_parse_result result = doc.load_file(fileName.c_str());

        if (!result)
        {
            string error = result.description();
            AgentUtils::writeLog(FILE_ERROR + fileName, FAILED);
            return FAILED;
        }

        pugi::xml_node root = doc.child("root");
        if (root)
        {
            for (pugi::xml_node groupNode = root.child("group"); groupNode; groupNode = groupNode.next_sibling("group"))
            {
                extractRuleAttributes(groupNode, table);
            }
        }
        else
        {
            root = doc.child("group");
            extractRuleAttributes(root, table);
        }
        
        AgentUtils::writeLog("XML parsing success for " + fileName, DEBUG);
        return SUCCESS;
    }

    /**
     * @brief Clean or Truncate a File
     *
     * The `cleanFile` function is used to truncate or clean the contents of a file located at the specified file path.
     * This operation removes all data from the file, effectively resetting it to an empty state.
     *
     * @param[in] filePath The file path of the file to be cleaned or truncated.
     * @return An integer result code:
     *         - SUCCESS: The file was successfully cleaned or truncated.
     *         - FAILED: The operation encountered errors and failed to clean or truncate the file.
     */
    int cleanFile(const string& filePath)
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

    /**
     * @brief Trim Leading and Trailing Whitespace from a String
     *
     * The `trim` function is used to remove leading and trailing whitespace characters from a given string, resulting in a
     * trimmed version of the string.
     *
     * @param[in] line The input string from which leading and trailing whitespace should be removed.
     * @return A string representing the trimmed version of the input string.
     */
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

    /**
     * @brief Convert Comma-Separated String to Vector
     *
     * The `toVector` function is used to convert a comma-separated string of fields into a vector of individual strings.
     *
     * @param[in] columns The input string containing comma-separated fields.
     * @param[in] sep The character used as the separator between fields (typically a comma ',' in this case).
     * @return A vector of strings, where each string represents an individual field from the input string.
     */
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

    /**
     * @brief Validate INI Configuration Line
     *
     * The `validateText` function is used to validate an INI configuration line to determine if it contains valid data or
     * should be considered as a comment or empty line. Valid lines are those that contain meaningful configuration data.
     *
     * @param[in,out] line The INI configuration line to be validated. If the line is not valid (comment or empty), it will
     *                    be modified to an empty string.
     * @return `true` if the line contains valid configuration data, `false` if it is a comment or empty line.
     */
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

    /**
     * @brief Read and Parse Decoder Configuration
     *
     * The `readDecoderConfig` function is used to read decoder configuration data from an XML file located at the specified
     * file path and parse it into a structured table. The parsed configuration data is organized as a map, where each decoder
     * is associated with a unique identifier and its configuration details.
     *
     * @param[in] path The file path to the XML decoder configuration file.
     * @param[out] table A reference to a map for storing the parsed decoder configuration. The map is structured as follows:
     *                  - The keys represent unique identifiers for each decoder.
     *                  - The values are instances of the `decoder` structure containing decoder-specific settings.
     * @return An integer result code:
     *         - SUCCESS: The XML decoder configuration file was successfully read and parsed into the table.
     *         - FAILED: The operation encountered errors and failed to read or parse the configuration.
     */
    int readDecoderConfig(const string& path, std::unordered_map<string, decoder>& table)
    {
        AgentUtils::writeLog("Reading " + path, DEBUG);
        if (!std::filesystem::is_regular_file(path))
        {
           AgentUtils::writeLog("Expected a file " + path, FATAL);
        }
        return parseToDecoder(path, table);
    }

    /**
     * @brief Read and Parse XML Rule Configuration
     *
     * The `readXmlRuleConfig` function is used to read IDS (Intrusion Detection System) rules in XML format from the specified
     * file path and parse them into a structured table. The parsed rules are organized in a `map` of rules, where each rule
     * is associated with a unique identifier and configuration details.
     *
     * @param[in] path The file path to the XML rule configuration file.
     * @param[in, out] table A reference to a map for storing the parsed IDS rules. The map is structured as follows:
     *                  - The keys are unique identifiers for each rule.
     *                  - The values are maps containing rule configuration settings.
     * @return An integer result code:
     *         - SUCCESS: The XML rule configuration was successfully read and parsed into the table.
     *         - FAILED: The operation encountered errors and failed to read or parse the configuration.
     */
    int readXmlRuleConfig(const string path, std::unordered_map<string, std::unordered_map<int, AConfig>> &table)
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

    /**
     * @brief Read and Parse INI Configuration File
     *
     * The `readIniConfigFile` function is used to read configuration settings from an INI format file located at the specified
     * file path and parse them into a structured table format. The parsed configuration settings are organized as a map of maps,
     * providing a convenient way to access and manage configuration data.
     *
     * @param[in] path The file path to the INI configuration file.
     * @param[in, out] table A reference to a map for storing the parsed configuration settings. The map is structured as follows:
     *                  - The keys represent sections in the INI file.
     *                  - The values are maps containing key-value pairs of configuration settings within each section.
     * @return An integer result code:
     *         - SUCCESS: The INI configuration file was successfully read and parsed into the table.
     *         - FAILED: The operation encountered errors and failed to read or parse the configuration.
     */
    int readIniConfigFile(const string path, map<string, map<string, string>> &table)
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

    /**
     * @brief Destructor for Config
     *
     * The destructor for the `Config` class performs cleanup and resource release when an instance of the class is
     * destroyed. It ensures that any allocated resources are properly released.
     */
    ~Config() {}
};

#endif