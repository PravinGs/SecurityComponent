#ifndef LOGPROXY_HPP
#define LOGPROXY_HPP

#include "service/config_service.hpp"
#include "model/patch_model.hpp"
#include "model/analysis_model.hpp"
#include "model/process_model.hpp"
#include "model/mqtt_model.hpp"

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
    string extract_file_name_from_url(const string &url)
    {
        size_t last_slash_index = url.find_last_of("/");
        if (last_slash_index != string::npos)
        {
            return url.substr(last_slash_index + 1);
        }
        return "";
    }

public:
    Proxy() = default;

    bool validate_log_entity(log_entity &entity)
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
                throw std::invalid_argument("proxy: validate_log_entity: no Specific time mentioned to collect log");
            }

            if (entity.format == "applog" && entity.columns.size() == 0)
            {
                throw std::invalid_argument("proxy: validate_log_entity: log attributes not configured for " + entity.name);
            }

            return true;
        }
        catch (exception &e)
        {
            string error = e.what();
            agent_utils::write_log("proxy: validate_log_entity: " + error, FAILED);
        }
        return false;
    }

    bool validate_analysis_entity(analysis_entity &entity)
    {
        if (!entity.log_path.empty() && !os::is_file_exist(entity.log_path))
        {
            agent_utils::write_log("proxy: validate_analysis_entity: invalid log path configured log analysis: " + entity.log_path, FAILED);
            return false;
        }

        if (!entity.decoder_path.empty() && !os::is_file_exist(entity.decoder_path))
        {
            agent_utils::write_log("proxy: validate_analysis_entity: invalid decoder path configured: " + entity.decoder_path, FAILED);
            return false;
        }
        else
        {
            entity.decoder_path = IDS_DEFAULT_DECODER_RULES_PATH;
        }

        if (!entity.write_path.empty() && !os::is_dir_exist(entity.write_path))
        {
            agent_utils::write_log("proxy: validate_analysis_entity: configured analyis write path not exist default path set", WARNING);
        }

        if (!entity.rules_path.empty() && !os::is_dir_exist(entity.rules_path))
        {
            agent_utils::write_log("proxy: validate_analysis_entity: invalid xml-rules path configured: " + entity.rules_path, FAILED);
            return false;
        }
        else
        {
            entity.rules_path = IDS_DEFAULT_XML_RULES_PATH;
        }

        if (entity.storage_type.empty())
        {
            entity.storage_type = "json";
        }
        return true;
    }

    bool validate_Rest_entity()
    {
        return true;
    }
    bool validate_patch_entity(patch_entity &entity)
    {
        if (entity.application.empty())
        {
            agent_utils::write_log("proxy: validate_patch_entity: application name not configured for patch update.", FAILED);
            return false;
        }

        if (entity.application_root_path.empty())
        {
            agent_utils::write_log("proxy: validate_patch_entity: application binary path not configured", FAILED);
            return false;
        }

        if (entity.url.empty())
        {
            agent_utils::write_log("proxy: validate_patch_entity: url not configured for patch download", FAILED);
            return FAILED;
        }

        if (entity.ca_cert_path.empty())
        {
            entity.is_secure = false;
        }
        else
        {
            entity.is_secure = (os::is_file_exist(entity.ca_cert_path)) ? true : false;
        }

        if (entity.client_cert_path.empty())
        {
            entity.is_secure = false;
        }
        else
        {
            entity.is_secure = (os::is_file_exist(entity.client_cert_path)) ? true : false;
        }

        if (entity.username.empty() && entity.password.empty())
        {
            entity.is_sftp = false;
        }

        if (entity.retry_count == 0)
        {
            entity.retry_count = PATCH_DEFAULT_RETRY_COUNT;
        }

        if (entity.retry_time_out == 0)
        {
            entity.retry_time_out = PATCH_DEFAULT_RETRY_TIMEOUT;
        }

        if (entity.max_download_speed == 0)
        {
            entity.max_download_speed = PATCH_DEFAULT_MAX_DOWNLOAD_SPEED;
        }

        if (entity.min_download_speed == 0)
        {
            entity.min_download_speed = PATCH_DEFAULT_MIN_DOWNLOAD_SPEED;
        }

        if (!os::is_dir_exist(PATCH_FILE_DOWNLOAD_PATH))
        {
            os::create_dir(PATCH_FILE_DOWNLOAD_PATH);
        }

        entity.download_path = PATCH_FILE_DOWNLOAD_PATH + extract_file_name_from_url(entity.url);

        return true;
    }

    bool validate_process_entity(process_entity &entity)
    {
        if (!entity.write_path.empty() && os::is_dir_exist(entity.write_path))
        {
            agent_utils::write_log("proxy: validate_process_entity: configured write directory path not exist", FAILED);
            return false;
        }

        if (entity.storage_type.empty())
        {
            entity.storage_type = "json";
        }

        return true;
    }

    bool get_previous_log_read_time(log_entity &entity)
    {
        string file_path = os::get_path_or_backup_file_path(entity.read_path);
        if (file_path.size() == 0)
        {
            agent_utils::write_log("proxy: get_previous_log_read_time: invalid file " + entity.read_path, FAILED);
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
            agent_utils::write_log("proxy: get_previous_log_read_time: log reading directory not exists, creating new directory");
            string tmp_config_dir = BASE_CONFIG_DIR;
            if (!os::is_dir_exist(tmp_config_dir))
                os::create_dir(tmp_config_dir);
            tmp_config_dir += BASE_CONFIG_TMP;
            if (!os::is_dir_exist(tmp_config_dir))
                os::create_dir(tmp_config_dir);
        }
        std::ofstream temp_config_file(temp_config_path);
        if (!temp_config_file)
        {
            agent_utils::write_log("proxy: get_previous_log_read_time: failed to create file check it's permission " + temp_config_path, FAILED);
            return false;
        }
        fstream fp(file_path, std::ios::in | std::ios::binary);
        if (!fp)
        {
            agent_utils::write_log("proxy: get_previous_log_read_time: invalid Log file " + file_path, FAILED);
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

    bool validate_mqtt_entity(mqtt_entity &entity)
    {
        if (entity.conn_string.empty())
        {
            agent_utils::write_log("proxy: validate_mqtt_entity: connection string is empty", FAILED);
            return false;
        }

        if (entity.topics.empty())
        {
            agent_utils::write_log("proxy: validate_mqtt_entity: topics not configured", FAILED);
            return false;
        }

        if (!entity.ca_cert_path.empty() && !os::is_file_exist(entity.ca_cert_path))
        {
            agent_utils::write_log("proxy: validate_mqtt_entity: file not exist " + entity.ca_cert_path, FAILED);
            return false;
        }
        else
        {
            entity.is_secure = false;
        }

        // it is an optional but may be required if server wanted to verify the client
        if (!entity.client_cert_path.empty() && !os::is_file_exist(entity.client_cert_path))
        {
            agent_utils::write_log("proxy: validate_mqtt_entity: file not exist " + entity.client_cert_path, WARNING);
        }

        if (entity.client_id.empty())
        {
            entity.client_id = os::host_name;
        }
        return true;
    }

    ~Proxy() {}
};

#endif