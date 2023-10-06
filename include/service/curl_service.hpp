#pragma once 

#include "common.hpp"

/**
 * @brief Curl Handler
 *
 * The `CurlHandler` class is responsible for managing all API-related activities within the application. It serves as
 * a central component for making HTTP requests, handling responses, and interacting with external services through APIs.
 * This class plays a crucial role in facilitating communication with external systems and services.
 */
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

    /**
     * @brief Perform POST Request
     *
     * The `post` function is a static method used to perform a POST request to the server. It sends data to a specific URL
     * identified by `postUrl` and retrieves the response. The `formName` parameter specifies the specific identifier for
     * the URL to which data is posted, and the `logName` parameter indicates where to fetch the data for the POST request.
     *
     * @param[in] postUrl The URL to which the POST request is sent.
     * @param[in] formName The identifier for the specific form or endpoint on the server.
     * @param[in] logName The identifier specifying which log to be sent.
     * @return A long integer representing the result of the POST request, typically an HTTP status code.
     */
    static long post(const string& postUrl, const string& formName, const string& logName)
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
                    std::cout << "Response: " << response << "\n";
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