#include "service/fwservice.hpp"

void FService::updateCurl(CURL *curl)
{
    this->curl = curl;
}

void FService::setFileSize(long size)
{
    this->fileSize = size;
}

long FService::readFileSize(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    this->setFileSize(size);
    return size;
}

std::string extractFileName(const std::string &url)
{
    size_t lastSlashIndex = url.find_last_of("/");
    if (lastSlashIndex != std::string::npos)
    {
        return url.substr(lastSlashIndex + 1);
    }
    return "";
}

int FService::download(map<string, map<string, string>> configTable)
{
    int returnVal = SUCCESS;
    const string writePath = configTable["firmware"]["write_path"];
    const string url = configTable["firmware"]["url"];
    string fileName = extractFileName(url);
    long MAX_DOWNLOAD_SPEED = std::stol(configTable["firmware"]["max_download_speed"]);
    long MIN_DOWNLOAD_SPEED = std::stol(configTable["firmware"]["min_download_speed"]);
    long TIME_OUT = std::stol(configTable["firmware"]["time_out"]);

    string downloadPath = writePath + fileName;

    FILE *file = NULL;
    long existingFileSize = fileSize;
    CURLcode res;
    int retry_count = 3;

    file = fopen(downloadPath.c_str(), "ab");

    if (file == NULL)
    {
        AgentUtils::writeLog("Failed to open " + downloadPath + " check file path and it's permission", FAILED);
        return FAILED;
    }

    existingFileSize = this->readFileSize(file);

    cout << "Existing file size " << existingFileSize << endl;

    curl = curl_easy_init();

    if (curl == NULL)
    {
        AgentUtils::writeLog("Failed to initialize curl ", FAILED);
        return FAILED;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, existingFileSize);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, MIN_DOWNLOAD_SPEED);
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, MAX_DOWNLOAD_SPEED);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, TIME_OUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OUT);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    do
    {
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            string error = curl_easy_strerror(res);
            AgentUtils::writeLog(error, FAILED);
            std::cerr << "Error: " << error << std::endl;
            retry_count--;
            if (retry_count > 0)
            {
                std::cerr << "Retrying download in 10 seconds..." << std::endl;
                AgentUtils::writeLog("Retrying download in 10 seconds...");
            }
            else
            {
                returnVal = SERVER_ERROR;
            }
        }
    } while (res != CURLE_OK && retry_count > 0);
    curl_easy_cleanup(curl);
    if (curl == NULL)
    {
        AgentUtils::writeLog("Download Stopped");
    }

    this->updateCurl(nullptr);
    fclose(file);
    return returnVal;
}