#ifndef LOGPROXY_HPP
#define LOGPROXY_HPP

#include "service/configservice.hpp"

int ORG_ID = 234225;

class CurlHandler
{
public:
    static size_t writeCallback(char *data, size_t size, size_t nmemb, std::string *response)
    {
        if (response)
        {
            response->append(data, size * nmemb);
            return size * nmemb;
        }
        return 0;
    }

    static size_t headerCallback(char *buffer, size_t size, size_t nitems, long *httpCode)
    {
        std::string header(buffer, size * nitems);
        size_t pos = header.find("HTTP/1.");
        if (pos != std::string::npos)
        {
            *httpCode = std::stoi(header.substr(pos + 9, 3));
        }
        return size * nitems;
    }

    static size_t verboseCallback(char *data, size_t size, size_t nmemb, std::string *output)
    {
        if (output)
        {
            output->append(data, size * nmemb);
            return size * nmemb;
        }
        return 0;
    }

    static long post(const string postUrl, const string formName, const string logName)
    {
        CURLcode res;
        const char *contentType = "application/json";
        std::string response;
        long httpCode = 0;
        curl_global_init(CURL_GLOBAL_DEFAULT);

        CURL *curl = curl_easy_init();
        vector<string> jsonFiles;
        int result = OS::readRegularFiles(jsonFiles);
        if (result == FAILED)
        {
            AgentUtils::writeLog(FILE_ERROR + logName, FAILED);
            return FAILED;
        }

        if (curl)
        {
            // Set the target URL
            curl_easy_setopt(curl, CURLOPT_URL, postUrl.c_str());

            // Set the request headers
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, "accept: */*");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Set the request as a POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            for (string jsonFile : jsonFiles)
            {

                // Set the request as multipart/form-data
                struct curl_httppost *form = nullptr;
                struct curl_httppost *last = nullptr;
                curl_formadd(&form, &last, CURLFORM_COPYNAME, formName.c_str(), CURLFORM_FILE, jsonFile.c_str(), CURLFORM_CONTENTTYPE, contentType, CURLFORM_END);
                curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
                curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpCode);

                // Enable verbose output
                // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

                res = curl_easy_perform(curl);
                if (res == CURLE_OK)
                {
                    AgentUtils::writeLog("Request successful");
                    AgentUtils::writeLog("HTTP Status Code: " + std::to_string(httpCode), SUCCESS);
                    std::cout << "Response: " << response << std::endl;
                }
                else
                {
                    string error = curl_easy_strerror(res);
                    AgentUtils::writeLog("Request failed: " + error, FAILED);
                }

                if (httpCode == POST_SUCCESS)
                {
                    OS::deleteFile(jsonFile);
                }
                else
                {
                    AgentUtils::writeLog("Failed to send this file " + jsonFile, FAILED);
                }
                std::chrono::seconds sleepDuration(1);
                std::this_thread::sleep_for(sleepDuration);
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        return httpCode;
    }
};

class Proxy
{
private:
    IniConfig _configService;

public:
    Proxy() = default;

    bool directoryExists(const string &directoryPath)
    {
        return std::filesystem::is_directory(directoryPath);
    }

    int isValidLogConfig(map<string, map<string, string>> &configTable, Json::Value &json, const string& name, char &remote, const string& prevTime)
    {
        int result = SUCCESS;
        try
        {
            string hostName = "unknown";

            vector<string> names = _configService.toVector(configTable[name]["columns"], ',');

            if (name != "syslog" && directoryExists(configTable[name]["log_directory"]))
            {
                throw std::invalid_argument("Invalid log directory for " + name);
            };

            if (names.size() == 0)
            {
                throw std::invalid_argument("Log attributes not configured for " + name);
            }

            if (configTable[name]["remote"].length() == 1)
            {
                remote = configTable[name]["remote"][0];
            }

            // else if (configTable[name]["remote"].length() > 1) { throw std::invalid_argument("Delimeter not configured Properly"); }

            if (prevTime.length() == 0)
            {
                throw std::invalid_argument("No Specific time mentioned to collect log");
            }

            if (AgentUtils::getHostName(hostName) == FAILED)
                return FAILED;

            json["OrgId"] = ORG_ID;
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
        fstream file(name, std::ios::in);
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

    long post(const string& postUrl, const string& formName, const string& jsonFile)
    {
        return CurlHandler::post(postUrl, formName, jsonFile);
    }

    ~Proxy() {}
};

#endif