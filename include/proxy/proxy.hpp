#ifndef LOGPROXY_HPP
#define LOGPROXY_HPP

#include "service/config_service.hpp"
#include "service/entity.hpp"

/**
 * @brief Proxy Validation and Control
 *
 * The `Proxy` class serves as the validation and control component for the agent application. It is responsible for
 * validating various aspects of the application's functionality and ensuring that the agent operates within predefined
 * criteria and security constraints. This class plays a crucial role in maintaining the integrity and security of the
 * agent's activities.
 */
class Proxy
{
private:
    Config _config_service; /**< A private instance of IniConfig for configuration management. */
public:
    Proxy() = default;


    bool validate_log_entity(log_entity& entity)
    {
        try
        {

            if (!entity.columns.empty())
            {
                entity.json_attributes = _config_service.to_vector(entity.columns, ',');
            }

            // if (entity.name != "syslog" && os::is_dir_exist(config_table[entity.name]["log_directory"]))
            // {
            //     throw std::invalid_argument("Invalid log directory for " + entity.name);
            // };

             if (entity.last_read_time == std::numeric_limits<time_t>::min())
            {
                throw std::invalid_argument("No Specific time mentioned to collect log");
            }

            if (entity.format == "applog" && entity.columns.size() == 0)
            {
                throw std::invalid_argument("Log attributes not configured for " + entity.name);
            }

            return true;
        }
        catch (exception &e)
        {
            string error = e.what();
            agent_utils::write_log(error, FAILED);
        }
        return false;
    }

    /**
     * @brief Validate Log Configuration
     *
     * The `isValidLogConfig` function validates the configuration for syslog, application log, or similar log types.
     * It uses the `config_table` parameter to access and validate the log configuration settings. The `json` parameter
     * is utilized to set common information about the logs. The `name` parameter specifies the log's identifier.
     * The `remote` parameter is used to identify whether a remote connection is configured or not.
     * The `prevTime` parameter is used to obtain the last read time of the log file.
     *
     * @param[in] config_table A map containing log configuration data.
     *                       The map should be structured to include settings for the specified log type.
     * @param[in, out] json A JSON object used to set common log information.
     * @param[in] name The identifier of the log being validated.
     * @param[in, out] remote A character indicating whether a remote connection is established ('Y' for yes, 'N' for no).
     * @param[in] prevTime The last read time of the log file.
     * @return An integer result code:
     *         - SUCCESS: The log configuration is valid.
     *         - FAILED: The validation encountered errors or the configuration is invalid.
     */

    int isValidLogConfig(map<string, map<string, string>> &config_table, Json::Value &json, const string& name, char &remote, const string& prevTime)
    {
        int result = SUCCESS;
        try
        {
            string hostName = "unknown";

            vector<string> names = _config_service.to_vector(config_table[name]["columns"], ',');

            if (name != "syslog" && os::is_dir_exist(config_table[name]["log_directory"]))
            {
                throw std::invalid_argument("Invalid log directory for " + name);
            };

            if (names.size() == 0)
            {
                throw std::invalid_argument("Log attributes not configured for " + name);
            }

            if (config_table[name]["remote"].length() == 1)
            {
                remote = (config_table[name]["remote"][0] == '1') ? 'y' : 'n';
            }

            // else if (config_table[name]["remote"].length() > 1) { throw std::invalid_argument("Delimeter not configured Properly"); }

            if (prevTime.length() == 0)
            {
                throw std::invalid_argument("No Specific time mentioned to collect log");
            }

            if (agent_utils::get_hostname(hostName) == FAILED)
                return FAILED;

            json["OrgId"] = 234225;
            json["AppName"] = config_table[name]["name"];
            json["Source"] = hostName;
        }
        catch (exception &e)
        {
            result = FAILED;
            string error = e.what();
            agent_utils::write_log(error, FAILED);
        }
        return result;
    }

    /**
     * @brief Get Last Log Written Time
     *
     * The `getLastLogWrittenTime` function is used to extract the last written time of a log file specified by its `name`
     * and `path`. This information is essential for tracking when the log file was last read to avoid reading duplicates.
     *
     * @param[in] name The identifier of the log file.
     * @param[in] path The path to the log file.
     * @return A string representing the last written time of the log file, formatted according to a specific timestamp format.
     *         If the operation fails, an empty string is returned.
     */
    bool get_previous_log_read_time(log_entity& entity)
    {
        string file_path = os::get_path_or_backup_file_path(entity.read_path);
        if (file_path.size() == 0)
        {
            agent_utils::write_log("Invalid file " + entity.read_path, FAILED);
            return false;
        }
        string temp_config_path = BASE_CONFIG_DIR;
        temp_config_path += BASE_CONFIG_TMP + entity.name;
        fstream file(temp_config_path, std::ios::in);
        string last_time = "";
        if (file.is_open())
        {
            std::getline(file, last_time);
            file.close();
            if (last_time.size() == 19)
            {
                entity.last_read_time = agent_utils::string_to_time_t(last_time);
                return true;
            }
        }
        else
        {
            agent_utils::write_log("Log reading directory not exists, creating new directory");
            string tmp_config_dir = BASE_CONFIG_DIR;
            if (os::is_dir_exist(tmp_config_dir) == FAILED)
                os::create_dir(tmp_config_dir);
            tmp_config_dir += BASE_CONFIG_TMP;
            if (os::is_dir_exist(tmp_config_dir) == FAILED)
                os::create_dir(tmp_config_dir);
        }
        std::ofstream temp_config_file(temp_config_path);
        if (!temp_config_file)
        {
            agent_utils::write_log("Failed to create file check it's permission " + temp_config_path, FAILED);
            return false;
        }
        fstream fp(file_path, std::ios::in | std::ios::binary);
        if (!fp)
        {
            agent_utils::write_log("Invalid Log file " + file_path, FAILED);
            temp_config_file.close();
            return false;
        }
        string line;
        std::getline(fp, line);
        if (entity.name == "syslog" || entity.name == "auth")
        {
            string timestamp = line.substr(0, 15);
            agent_utils::convert_time_format(timestamp, last_time);
        }
        else if (entity.name == "dpkg")
        {
            last_time = line.substr(0, 19);
        }
        temp_config_file << last_time;
        fp.close();
        temp_config_file.close();
        entity.last_read_time = agent_utils::string_to_time_t(last_time);
        return true;
    }

    /**
     * @brief Destructor for Proxy.
     *
     * The destructor performs cleanup tasks for the `Proxy` class, which may include
     * releasing resources and deallocating memory.
     */
    ~Proxy() {}
};

#endif