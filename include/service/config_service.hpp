#ifndef CONFIGSERVICE_HPP
#define CONFIGSERVICE_HPP
#pragma once

#include "common.hpp"
#include "entity/analysis_entity.hpp"
#include "entity/agent_entity.hpp"
#include "entity/log_entity.hpp"
#include "entity/ids_entity.hpp"
#include "entity/process_entity.hpp"
#include "entity/api_entity.hpp"

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

    /**
     * @brief Convert String to Integer with Error Handling
     *
     * The `is_digit` function is used to convert a string representing a number into an integer, while also providing error
     * handling for cases where the conversion fails due to invalid input.
     *
     * @param[in] number The input string to be converted to an integer.
     * @return An integer result code:
     *         - The converted integer value if the conversion was successful.
     *         - -1 if the input string is not a valid integer or conversion fails.
     */
    int is_digit(const string&  number)
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

    int read_decoder(const string & file_name, std::unordered_map<string, decoder>& table)
    {   
        /*
            for (const auto& d: decoders)
            {
                decoder s = d.second;
                cout << "<decoder name=" << d.first << " >\n";
                cout << "    <parent> " << s.parent << " </parent>\n";
                cout << "    <program_name_pcre2> " << s.program_name_pcre2 << " </program_name_pcre2>\n";
                cout << "    <pcre2> " << s.pcre2 << " </pcre2>\n";
                cout << "    <prematch_pcre2> " << s.prematch_pcre2 << " </prematch_pcre2>\n";
                cout << "    <order> " << s.order << " </order>\n";
                cout << "   <prematch_offset> " << s.prematch_offset << " </prematch_offset>\n";
                cout << "   <pcre2_offset> " << s.pcre2_offset << " </pcre2_offset>\n";
            }
        */     
        pugi::xml_parse_result result = doc.load_file(file_name.c_str());
        int index = 0;
        if (!result)
        {
            string error = result.description();
            agent_utils::write_log(FILE_ERROR + file_name, FAILED);
            return FAILED;
        }
        pugi::xml_node root = doc.child("root");
        for (pugi::xml_node group_node = root.child("decoder"); group_node; group_node = group_node.next_sibling("decoder"))
        {
            index++;
            decoder d;
            std::string current_section = group_node.attribute("name").value();
            if (!current_section.empty())
            {
                d.decode=current_section;
            }
            current_section = group_node.child_value("parent");
            if (!current_section.empty())
            {
                d.parent=current_section;
            }
            current_section = group_node.child_value("program_name_pcre2");
            if (!current_section.empty())
            {
                d.program_name_pcre2=current_section;
            }
            if (current_section == "^kernel") { cout << d.decode << "\n";}
            current_section = group_node.child_value("pcre2");
            if (!current_section.empty())
            {
                d.pcre2=current_section;
            }
            current_section = group_node.child("pcre2").attribute("offset").value();
            if (!current_section.empty())
            {
                d.pcre2_offset=current_section;
            }
            current_section = group_node.child_value("prematch_pcre2");
            if (!current_section.empty())
            {
                d.prematch_pcre2=current_section;
            }
            current_section = group_node.child("prematch_pcre2").attribute("offset").value();
            if (!current_section.empty())
            {
                d.prematch_offset=current_section;
            }
            current_section = group_node.child_value("order");
            if (!current_section.empty())
            {
                d.order=current_section;
            }
            current_section = group_node.child_value("fts");
            if (!current_section.empty())
            {
                d.fts=current_section;
            }
            if (table.find(d.decode) != table.end())
            {
                table.at(d.decode).update(d);
            }
            else 
            {
                table[d.decode] = d;
            }
            
            // cout << "Index [" << sec << "] : " << index << "  size : " << table.size() << "\n";
            
        }
        agent_utils::write_log("XML parsing success for " + file_name, DEBUG);
        return SUCCESS;
    }

    void extract_rule_attributes(pugi::xml_node & root, std::unordered_map<string, std::unordered_map<int, aconfig>> &table)
    {
        for (pugi::xml_node group_node = root; group_node; group_node = group_node.next_sibling("group"))
        {
            std::string current_section = group_node.attribute("name").value();

            for (pugi::xml_node rule_node = group_node.child("rule"); rule_node; rule_node = rule_node.next_sibling("rule"))
            {
                int digit;
                string str;
                aconfig rule;
                if (!current_section.empty())
                {
                    rule.group = current_section; /*Need clarity about this group assignation*/
                }
                digit = is_digit(rule_node.attribute("id").value());
                if (digit != -1)
                {
                    rule.id = digit;
                }
                digit = -1;

                digit = is_digit(rule_node.attribute("level").value());
                if (digit != -1)
                {
                    rule.level = digit;
                }
                digit = -1;
                digit = is_digit(rule_node.attribute("frequency").value());
                if (digit != -1)
                {
                    rule.frequency = digit;
                }
                digit = -1;
                digit = is_digit(rule_node.attribute("timeframe").value());
                if (digit != -1)
                {
                    rule.timeframe = digit;
                }
                digit = -1;
                digit = is_digit(rule_node.attribute("ignore").value());
                if (digit != -1)
                {
                    rule.ignore = digit;
                }
                digit = -1;

                digit = is_digit(rule_node.child_value("if_matched_sid"));
                // cout << "Match Id: " << digit;
                if (digit != -1)
                {
                    rule.if_matched_id = digit;
                }
                digit = -1;

                digit = is_digit(rule_node.child_value("if_sid"));
                if (digit != -1)
                {
                    rule.if_sid = digit;
                }
                digit = -1;
                digit = is_digit(rule_node.child_value("same_source_ip"));
                if (digit != -1){
                    rule.same_source_ip = 1;
                }
                digit = -1;
                str = rule_node.child_value("status_pcre2");
                if (!str.empty())
                {
                    rule.status_pcre2 = str;
                }

                str = rule_node.child_value("id_pcre2");
                if (!str.empty())
                {
                    rule.id_pcre2 = str;
                }

                str = rule_node.child_value("extra_data_pcre2");
                if (!str.empty())
                {
                    rule.extra_data_pcre2 = str;
                }

                // Iterate through the child nodes of <rule> to find <pcre2> elements

                /*str = rule_node.child_value("pcre2");
                if (!str.empty())
                {
                    rule.pcre2 = str;
                }*/

                str = rule_node.child_value("program_name_pcre2");
                if (!str.empty())
                {
                    rule.program_name_pcre2 = str;
                }

                str = rule_node.child_value("scrip"); /*Not sure */
                if (!str.empty())
                {
                    rule.script = str;
                }

                str = rule_node.child_value("group");
                if (!str.empty())
                {
                    rule.group = str;
                }

                str = rule_node.child_value("description");
                if (!str.empty())
                {
                    rule.description = str;
                }

                str = rule_node.child_value("decoded_as");
                if (!str.empty())
                {
                    rule.decoded_as = str;
                }

                str = rule_node.child_value("category"); /*check*/
                if (!str.empty())
                {
                    rule.categories = str;
                }

                str = rule_node.child_value("action");
                if (!str.empty())
                {
                    rule.action = str;
                }

                str = rule_node.child_value("options");
                if (!str.empty())
                {
                    rule.options = str;
                }

                str = rule_node.child_value("url_pcre2");
                if (!str.empty())
                {
                    rule.url_pcre2 = str;
                }

                str = rule_node.child_value("compiled_rule");
                if (!str.empty())
                {
                    rule.compiled_rule = str;
                }

                str = rule_node.child_value("hostname_pcre2");
                if (!str.empty())
                {

                    rule.hostname_pcre2 = str;
                }
                for (pugi::xml_node pcre2_node = rule_node.child("pcre2"); pcre2_node; pcre2_node = pcre2_node.next_sibling("pcre2")) 
                {
                    string s = pcre2_node.text().as_string();
                    if (!s.empty()) {
                        rule.pcre2.push_back(s); // Store each <pcre2> value in the vector
                    }
                }

                table[current_section][rule.id] = rule;
            }
        }
    }

    /**
     * @brief Parse XML Nodes into aconfig Table
     *
     * The `read_aconfig` function is used to parse XML nodes from a specified XML file and populate a structured table of
     * `aconfig` objects. Each XML node represents configuration data, and the parsed data is organized within the table.
     *
     * @param[in] file_name The file name of the XML file containing configuration nodes.
     * @param[out] table A reference to a map for storing the parsed configuration data. The map is structured as follows:
     *                  - The keys represent unique identifiers for each configuration.
     *                  - The values are maps containing configuration settings organized by integer keys.
     * @return An integer result code:
     *         - SUCCESS: The XML nodes were successfully parsed and populated into the aconfig table.
     *         - FAILED: The operation encountered errors and failed to parse or populate the configuration data.
     */
    int read_aconfig(string file_name, std::unordered_map<string, std::unordered_map<int, aconfig>> &table)
    {
        pugi::xml_parse_result result = doc.load_file(file_name.c_str());

        if (!result)
        {
            string error = result.description();
            agent_utils::write_log(FILE_ERROR + file_name, FAILED);
            return FAILED;
        }

        pugi::xml_node root = doc.child("root");
        if (root)
        {
            for (pugi::xml_node group_node = root.child("group"); group_node; group_node = group_node.next_sibling("group"))
            {
                extract_rule_attributes(group_node, table);
            }
        }
        else
        {
            root = doc.child("group");
            extract_rule_attributes(root, table);
        }
        
        agent_utils::write_log("XML parsing success for " + file_name, DEBUG);
        return SUCCESS;
    }

    /**
     * @brief Clean or Truncate a File
     *
     * The `clean_file` function is used to truncate or clean the contents of a file located at the specified file path.
     * This operation removes all data from the file, effectively resetting it to an empty state.
     *
     * @param[in] filePath The file path of the file to be cleaned or truncated.
     * @return An integer result code:
     *         - SUCCESS: The file was successfully cleaned or truncated.
     *         - FAILED: The operation encountered errors and failed to clean or truncate the file.
     */
    int clean_file(const string& filePath)
    {
        std::ofstream file(filePath, std::ios::trunc);
        if (file.is_open())
        {
            agent_utils::write_log(CLEAN_FILE + " <" + filePath + ">", SUCCESS);
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
     * The `to_vector` function is used to convert a comma-separated string of fields into a vector of individual strings.
     *
     * @param[in] columns The input string containing comma-separated fields.
     * @param[in] sep The character used as the separator between fields (typically a comma ',' in this case).
     * @return A vector of strings, where each string represents an individual field from the input string.
     */
    vector<string> to_vector(string columns, const char sep)
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
     * The `validate_ini_file_text` function is used to validate an INI configuration line to determine if it contains valid data or
     * should be considered as a comment or empty line. Valid lines are those that contain meaningful configuration data.
     *
     * @param[in,out] line The INI configuration line to be validated. If the line is not valid (comment or empty), it will
     *                    be modified to an empty string.
     * @return `true` if the line contains valid configuration data, `false` if it is a comment or empty line.
     */
    bool validate_ini_file_text(string &line)
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
     * The `read_decoder_config` function is used to read decoder configuration data from an XML file located at the specified
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
    int read_decoder_config(const string& path, std::unordered_map<string, decoder>& table)
    {
        agent_utils::write_log("Reading " + path, DEBUG);
        if (!std::filesystem::is_regular_file(path))
        {
           agent_utils::write_log("Expected a file " + path, FATAL);
        }
        return read_decoder(path, table);
    }

    /**
     * @brief Read and Parse XML Rule Configuration
     *
     * The `read_xml_rule_config` function is used to read IDS (Intrusion Detection System) rules in XML format from the specified
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
    int read_xml_rule_config(const string path, std::unordered_map<string, std::unordered_map<int, aconfig>> &table)
    {
        agent_utils::write_log("Reading " + path, DEBUG);
        int result = SUCCESS;
        int isFile = 0;
        if (os::is_exist(path) && std::filesystem::is_regular_file(path))
        {
            isFile = 1;
        }

        if (isFile)
        {
            result = read_aconfig(path, table);
        }
        else
        {
            string current_file = path;
            vector<string> files;
            if (current_file[current_file.size() - 1] == '/')
            {
                current_file = current_file.substr(0, current_file.find_last_of('/'));
            }
            result = os::get_regular_files(current_file, files);

            if (result == FAILED)
                return FAILED;

            if (files.size() == 0)
            {
                agent_utils::write_log(INVALID_PATH + path, FAILED);
                return FAILED;
            }
            for (string file : files)
            {
                result = read_aconfig(file, table);
            }
        }

        return result;
    }

    /**
     * @brief Read and Parse INI Configuration File
     *
     * The `rea_ini_config_file` function is used to read configuration settings from an INI format file located at the specified
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
    int read_ini_config_file(const string path, map<string, map<string, string>> &table)
    {
        int result = SUCCESS;
        fstream file(path, std::ios::in | std::ios::binary);
        string line, current_section;
        int index = 1;

        // if (!file)
        // {
        //     agent_utils::write_log(INVALID_FILE + path, FAILED);
        //     return FAILED;
        // }

        while (std::getline(file, line))
        {
            line = trim(line);

            if (line.empty() || line[0] == ';')
            {
                continue;
            }
            else if (line[0] == '[' && line[line.size() - 1] == ']')
            {
                current_section = trim(line.substr(1, line.size() - 2));
            }
            else if (line[0] == '[' && line[line.size() - 1] != ']')
            {
                agent_utils::write_log(INVALID_FILE + path, FAILED);
                agent_utils::write_log("Invalid Config file : line number " + std::to_string(index), FAILED);
                result = INVALID_CONFIGURATION;
                break;
            }
            else
            {
                size_t delimiter = line.find('=');
                if (delimiter != string::npos)
                {
                    string key = trim(line.substr(0, delimiter));
                    if (!validate_ini_file_text(key))
                    {
                        agent_utils::write_log("Invalid Config file : line number " + std::to_string(index), FAILED);
                        result = INVALID_CONFIGURATION;
                        break;
                    }
                    string value = trim(line.substr(delimiter + 1));
                    validate_ini_file_text(value);
                    table[current_section][key] = value;
                }
            }
            index++;
        }
        return result;
    }

    storage get_storage(const string& name, map<string, map<string, string>> & config_table)
    {
        storage stg;
        stg.url = config_table[name]["url"];
        stg.certificate = config_table[name]["certificate"];
        stg.user_name = config_table[name]["user_name"];
        stg.password = config_table[name]["password"];
        return stg;
    }

    //  log_collector::log_collector(const string &type, const string &read_path, char delimeter,
    //                              const string &write_path, vector<string> &commands, const string &time_pattern,
    //                              const storage &storage_type, const string &rest_url, const string &rest_attribute) : type(type), read_path(read_path), delimeter(delimeter), write_path(write_path), commands(commands), time_pattern(time_pattern), storage_type(storage_type), rest_url(rest_url), rest_attribute(rest_attribute)
    // {
    // }

    log_entity get_syslog_entity(map<string, map<string, string>> & config_table)
    {
        const string name = "syslog";
        vector<string> commands = to_vector(config_table[name]["commands"], ',');
        storage memory_type = get_storage(name, config_table);
        log_entity entity(
            name,
            config_table[name]["read_path"],
            config_table[name]["delimeter"],
            config_table[name]["write_path"],
            commands,
            config_table[name]["time_pattern"],
            memory_type,
            config_table[name]["rest_url"],
            config_table[name]["rest_attribute"]
        );
        return entity;
    }

    log_entity get_applog_entity(map<string, map<string, string>> & config_table)
    {
        const string name = "applog";
        vector<string> commands = to_vector(config_table[name]["commands"], ',');
        storage memory_type = get_storage(name, config_table);
        log_entity entity(
            name,
            config_table[name]["read_path"],
            config_table[name]["delimeter"],
            config_table[name]["write_path"],
            commands,
            config_table[name]["time_pattern"],
            memory_type,
            config_table[name]["rest_url"],
            config_table[name]["rest_attribute"]
        );
        return entity;
    }

   analysis_entity get_analysis_entity(map<string, map<string, string>> & config_table)
    {   
        const string name = "log_analysis";
        analysis_entity entity(
           config_table[name]["file_path"],
           config_table[name]["decoder_path"],
           config_table[name]["rules_dir"],
           config_table[name]["time_pattern"],
           get_storage(name, config_table),
           config_table[name]["rest_url"],
           config_table[name]["rest_attribute"]

        );
        return entity;
    }   

    process_entity get_process_entity(map<string, map<string, string>>& config_table)
    {
        const string name = "process";
        process_entity entity(
            config_table[name]["write_path"],
            config_table[name]["time_pattern"],
            get_storage(name, config_table),
            config_table[name]["rest_url"],
            config_table[name]["rest_attribute"]
        );
        return entity;
    }

    void create_agent_entity(map<string, map<string, string>> & config_table, agent_entity& entity)
    {
        log_entity syslog_entity = get_syslog_entity(config_table);
        log_entity applog_entity = get_applog_entity(config_table);
        analysis_entity an_entity = get_analysis_entity(config_table);
        process_entity p_entity = get_process_entity(config_table);
        entity.setSysLog(syslog_entity);
        entity.setAppLog(applog_entity);
        entity.setLogAnalysis(an_entity);
        entity.setProcess(p_entity);
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