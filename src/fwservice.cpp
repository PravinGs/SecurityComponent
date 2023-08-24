#include "service/fwservice.hpp"

FService::FService(){}

int FService::createDProps(map<string, map<string, string>> table)
{
    int result = SUCCESS;
    this->dProperties.writePath = table["firmware"]["write_path"];
    if (dProperties.writePath.empty()) 
    {
        AgentUtils::writeLog("Write path not defined", WARNING);
        result = FAILED;
    }
    dProperties.url = table["firmware"]["url"];
    if (dProperties.url.empty()) 
    {
        AgentUtils::writeLog("url path not defined", WARNING);
        result = FAILED;
    }
    dProperties.fileName = extractFileName(dProperties.url);
    if (dProperties.fileName.empty()) 
    {
        AgentUtils::writeLog("fileName path not defined", WARNING);
        result = FAILED;
    }
    dProperties.downloadPath = dProperties.writePath + dProperties.fileName;
    if (dProperties.downloadPath.empty()) 
    {
        AgentUtils::writeLog("downloadPath path not defined", WARNING);
        result = FAILED;
    }
    try
    {
        dProperties.maxSpeed = std::stoi(table["firmware"]["max_download_speed"]);
        dProperties.minSpeed = std::stoi(table["firmware"]["min_download_speed"]);
        dProperties.timeout = std::stoi(table["firmware"]["time_out"]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        result = FAILED;
    }
    return result;
}

long FService::readFileSize(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    return size;
}

string FService::extractFileName(const string &url)
{
    size_t lastSlashIndex = url.find_last_of("/");
    if (lastSlashIndex != string::npos)
    {
        return url.substr(lastSlashIndex + 1);
    }
    return "";
}


int FService::download()
{
    int returnVal = SUCCESS;
    FILE *file = NULL;
    CURLcode res;
    file = fopen(dProperties.downloadPath.c_str(), "ab");

    if (file == NULL)
    {
        AgentUtils::writeLog("Failed to open " + dProperties.downloadPath + " check file path and it's permission", FAILED);
        return FAILED;
    }

    dProperties.size = this->readFileSize(file);

    cout << "Existing file size " << dProperties.size << endl;

    if (curl == NULL)
    {
        AgentUtils::writeLog("Failed to initialize curl ", FAILED);
        return FAILED;
    }

    curl_easy_setopt(curl, CURLOPT_URL, dProperties.url.c_str());
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, dProperties.size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, dProperties.minSpeed);
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, dProperties.maxSpeed);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, dProperties.timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, dProperties.timeout);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    do
    {
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            string error = curl_easy_strerror(res);
            AgentUtils::writeLog(error, FAILED);
            std::cerr << "Error: " << error << std::endl;
            dProperties.retry--;
            if (dProperties.retry > 0)
            {
                std::cerr << "Retrying download in 10 seconds..." << std::endl;
                AgentUtils::writeLog("Retrying download in 10 seconds...");
            }
            else
            {
                returnVal = SERVER_ERROR;
            }
        }
    } while (res != CURLE_OK && dProperties.retry > 0);
    if (dProperties.retry <= 0)
    {
        returnVal = FAILED;
    }
    fclose(file);
    return returnVal;
}

int FService::start(map<string, map<string, string>> table)
{
    int result = SUCCESS;
    int count = 5;
    if (createDProps(table) == FAILED) { return FAILED; }
    curl = curl_easy_init();
    while(result != FAILED || count > 0)
    {
        if (result == TIME_OUT_ERROR)
        {
            std::this_thread::sleep_for(std::chrono::seconds(dProperties.timeout));
            count--;
        }        
        result = download();
    }
    if (curl != nullptr)
    {
        curl_easy_cleanup(curl);
    }
    if (count <= 0)
    {
        result = SCHEDULAR_WAIT;
        AgentUtils::writeLog("Reached time limit exiting from download", WARNING);
    }

    return result;
}

FService::~FService() 
{
}