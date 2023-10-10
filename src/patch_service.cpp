#include "service/patch_service.hpp"

patch_service::patch_service(){}

int patch_service::create_download_property(map<string, map<string, string>>& table)
{
    int result = SUCCESS;
    this->d_properties.writePath = table["firmware"]["write_path"];
    if (d_properties.writePath.empty()) 
    {
        agent_utils::write_log("Write path not defined", WARNING);
        result = FAILED;
    }
    this->d_properties.writePath = table["firmware"]["root_dir"];
    if (d_properties.writePath.empty()) 
    {
        agent_utils::write_log("Root directory not defined", WARNING);
        result = FAILED;
    }
    d_properties.url = table["firmware"]["url"];
    if (d_properties.url.empty()) 
    {
        agent_utils::write_log("url path not defined", WARNING);
        result = FAILED;
    }
    d_properties.fileName = extract_ile_name(d_properties.url);
    if (d_properties.fileName.empty()) 
    {
        agent_utils::write_log("fileName path not defined", WARNING);
        result = FAILED;
    }
    d_properties.downloadPath = d_properties.writePath + d_properties.fileName;
    if (d_properties.downloadPath.empty()) 
    {
        agent_utils::write_log("downloadPath path not defined", WARNING);
        result = FAILED;
    }
    username = table["firmware"]["username"];
    password = table["firmware"]["password"];
    try
    {
        d_properties.maxSpeed = std::stoi(table["firmware"]["max_download_speed"]);
        d_properties.minSpeed = std::stoi(table["firmware"]["min_download_speed"]);
        d_properties.timeout = std::stoi(table["firmware"]["time_out"]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        result = FAILED;
    }
    return result;
}

long patch_service::readFileSize(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    return size;
}

string patch_service::extract_ile_name(const string &url)
{
    size_t lastSlashIndex = url.find_last_of("/");
    if (lastSlashIndex != string::npos)
    {
        return url.substr(lastSlashIndex + 1);
    }
    return "";
}

int patch_service::download(const string& username, const string& password)
{
    int returnVal = SUCCESS;
    string credential = username + ":" + password;
    int retry = d_properties.retry;
    FILE *file = NULL;
    CURLcode res;
    file = fopen(d_properties.downloadPath.c_str(), "ab");

    if (file == NULL)
    {
        agent_utils::write_log("Failed to open " + d_properties.downloadPath + " check file path and it's permission", FAILED);
        return FAILED;
    }

    d_properties.size = this->readFileSize(file);

    if (curl == NULL)
    {
        agent_utils::write_log("Failed to initialize curl ", FAILED);
        return FAILED;
    }

    curl_easy_setopt(curl, CURLOPT_URL, d_properties.url.c_str());
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, d_properties.size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD); 
    curl_easy_setopt(curl, CURLOPT_USERPWD, credential.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, d_properties.minSpeed);
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, d_properties.maxSpeed);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, d_properties.timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, d_properties.timeout);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    do
    {
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK)
        {
            string error = curl_easy_strerror(res);
            agent_utils::write_log(error, FAILED);
            std::cerr << "Error: " << error << "\n";
            if (res == CURLE_COULDNT_RESOLVE_HOST)
            {
                returnVal = FAILED;
                break;
            }
            retry--;
            if (retry > 0)
            {
                std::cerr << "Retrying download in 10 seconds..." << "\n";
                agent_utils::write_log("Retrying download in 10 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            else
            {
                returnVal = SERVER_ERROR;
            }
        }
        else
        {
            retry = d_properties.retry;
        }
    } while (res != CURLE_OK && retry > 0);
    if (d_properties.retry <= 0)
    {
        returnVal = FAILED;
    }
    fclose(file);
    return returnVal;
}

int patch_service::download()
{
    int returnVal = SUCCESS;
    int retry = d_properties.retry;
    FILE *file = NULL;
    CURLcode res;
    file = fopen(d_properties.downloadPath.c_str(), "ab");

    if (file == NULL)
    {
        agent_utils::write_log("Failed to open " + d_properties.downloadPath + " check file path and it's permission", FAILED);
        return FAILED;
    }

    d_properties.size = this->readFileSize(file);

    if (curl == NULL)
    {
        agent_utils::write_log("Failed to initialize curl ", FAILED);
        return FAILED;
    }

    curl_easy_setopt(curl, CURLOPT_URL, d_properties.url.c_str());
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, d_properties.size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, d_properties.minSpeed);
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, d_properties.maxSpeed);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, d_properties.timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, d_properties.timeout);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    do
    {
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK)
        {
            string error = curl_easy_strerror(res);
            agent_utils::write_log(error, FAILED);
            std::cerr << "Error: " << error << "\n";
            if (res == CURLE_COULDNT_RESOLVE_HOST)
            {
                returnVal = FAILED;
                break;
            }
            retry--;
            if (retry > 0)
            {
                std::cerr << "Retrying download in 10 seconds..." << "\n";
                agent_utils::write_log("Retrying download in 10 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            else
            {
                returnVal = SERVER_ERROR;
            }
        }
        else
        {
            retry = d_properties.retry;
        }
    } while (res != CURLE_OK && retry > 0);
    if (d_properties.retry <= 0)
    {
        returnVal = FAILED;
    }
    fclose(file);
    return returnVal;
}

int patch_service::start(map<string, map<string, string>>& table)
{
    int result = SUCCESS;
    int count = 5;
    if (create_download_property(table) == FAILED) { return FAILED; }
    curl = curl_easy_init();
    while(count > 0)
    {     
        if (!username.empty() && !password.empty())   
        {
            result = download(username, password);
        }
        else
        {
            result = download();
        }
        if (result == SUCCESS)
        {
            agent_utils::write_log("Download completed..", SUCCESS);
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(d_properties.timeout));
            count--;
        }
    }
    if (curl != nullptr)
    {
        curl_easy_cleanup(curl);
    }
    if (count <= 0)
    {
        result = SCHEDULAR_WAIT;
        agent_utils::write_log("Reached time limit exiting from download", WARNING);
    }

    return result;
}

patch_service::~patch_service() 
{
}