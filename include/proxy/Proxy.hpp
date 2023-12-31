#ifndef LOGPROXY_HPP
#define LOGPROXY_HPP

#include "service/configservice.hpp"

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
    Config _configService; /**< A private instance of IniConfig for configuration management. */
public:
    Proxy() = default;

    /**
     * @brief Validate Log Configuration
     *
     * The `isValidLogConfig` function validates the configuration for syslog, application log, or similar log types.
     * It uses the `configTable` parameter to access and validate the log configuration settings. The `json` parameter
     * is utilized to set common information about the logs. The `name` parameter specifies the log's identifier.
     * The `remote` parameter is used to identify whether a remote connection is configured or not.
     * The `prevTime` parameter is used to obtain the last read time of the log file.
     *
     * @param[in] configTable A map containing log configuration data.
     *                       The map should be structured to include settings for the specified log type.
     * @param[in, out] json A JSON object used to set common log information.
     * @param[in] name The identifier of the log being validated.
     * @param[in, out] remote A character indicating whether a remote connection is established ('Y' for yes, 'N' for no).
     * @param[in] prevTime The last read time of the log file.
     * @return An integer result code:
     *         - SUCCESS: The log configuration is valid.
     *         - FAILED: The validation encountered errors or the configuration is invalid.
     */

    int isValidLogConfig(map<string, map<string, string>> &configTable, Json::Value &json, const string& name, char &remote, const string& prevTime)
    {
        int result = SUCCESS;
        try
        {
            string hostName = "unknown";

            vector<string> names = _configService.toVector(configTable[name]["columns"], ',');

            if (name != "syslog" && OS::isDirExist(configTable[name]["log_directory"]))
            {
                throw std::invalid_argument("Invalid log directory for " + name);
            };

            if (names.size() == 0)
            {
                throw std::invalid_argument("Log attributes not configured for " + name);
            }

            if (configTable[name]["remote"].length() == 1)
            {
                remote = (configTable[name]["remote"][0] == '1') ? 'y' : 'n';
            }

            // else if (configTable[name]["remote"].length() > 1) { throw std::invalid_argument("Delimeter not configured Properly"); }

            if (prevTime.length() == 0)
            {
                throw std::invalid_argument("No Specific time mentioned to collect log");
            }

            if (AgentUtils::getHostName(hostName) == FAILED)
                return FAILED;

            json["OrgId"] = 234225;
            json["AppName"] = configTable[name]["name"];
            json["Source"] = hostName;
        }
        catch (exception &e)
        {
            result = FAILED;
            string error = e.what();
            AgentUtils::writeLog(error, FAILED);
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
    string getLastLogWrittenTime(const string& name, const string& path)
    {
        string nonEmtPath = OS::isEmpty(path);
        if (nonEmtPath.size() == 0)
        {
            AgentUtils::writeLog("Invalid file " + path, FAILED);
            return "";
        }
        string filePath = BASE_CONFIG_DIR;
        filePath += BASE_CONFIG_TMP + name;
        fstream file(filePath, std::ios::in);
        string lastTime = "";
        if (file.is_open())
        {
            std::getline(file, lastTime);
            file.close();
            if (lastTime.size() == 19)
            {
                return lastTime;
            }
        }
        else
        {
            AgentUtils::writeLog("Log reading directory not exists, creating new directory");
            string dirPath = BASE_CONFIG_DIR;
            if (OS::isDirExist(dirPath) == FAILED)
                OS::createDir(dirPath);
            dirPath += BASE_CONFIG_TMP;
            if (OS::isDirExist(dirPath) == FAILED)
                OS::createDir(dirPath);
        }
        std::ofstream nf(filePath);
        if (!nf)
        {
            AgentUtils::writeLog("Failed to create file check it's permission", FAILED);
            return lastTime;
        }
        fstream fp(nonEmtPath, std::ios::in | std::ios::binary);
        if (!fp)
        {
            AgentUtils::writeLog("Invalid Log file " + path, FAILED);
            nf.close();
            return lastTime;
        }
        string line;
        std::getline(fp, line);
        if (name == "syslog" || name == "auth")
        {
            string timestamp = line.substr(0, 15);
            AgentUtils::convertTimeFormat(timestamp, lastTime);
        }
        else if (name == "dpkg")
        {
            lastTime = line.substr(0, 19);
        }
        nf << lastTime;
        fp.close();
        nf.close();
        return lastTime;
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