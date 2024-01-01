#include "service/patch_service.hpp"

patch_service::patch_service() {}

long patch_service::seek_file_size(FILE *file)
{
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    return size;
}

int patch_service::download(patch_entity &entity)
{
    int return_val = SUCCESS;
    int retry = entity.retry_time_out;
    FILE *file = NULL;
    CURLcode res;

    file = fopen(entity.download_path.c_str(), "ab");
    if (file == NULL)
    {
        agent_utils::write_log("Failed to open " + entity.download_path + " check file path and it's permission", FAILED);
        return FAILED;
    }

    entity.size = seek_file_size(file);

    if (curl == NULL)
    {
        agent_utils::write_log("Failed to initialize curl ", FAILED);
        return FAILED;
    }

    curl_easy_setopt(curl, CURLOPT_URL, entity.url.c_str());
    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, entity.size);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (entity.is_sftp)
    {
        string credential = entity.username + ":" + entity.password;
        curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_USERPWD, credential.c_str());
    }

    if (!entity.is_secure)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    else 
    {
        curl_easy_setopt(curl, CURLOPT_CAINFO, entity.ca_cert_path.c_str());
        if (!entity.client_cert_path.empty())
        {
            curl_easy_setopt(curl, CURLOPT_SSLCERT, entity.client_cert_path.c_str());
        }
    }

    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, entity.min_download_speed);
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, entity.max_download_speed);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, entity.retry_time_out);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, entity.retry_time_out);
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
                return_val = FAILED;
                break;
            }
            retry--;
            if (retry > 0)
            {
                std::cerr << "Retrying download in 10 seconds..."
                          << "\n";
                agent_utils::write_log("Retrying download in 10 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            else
            {
                return_val = SERVER_ERROR;
            }
        }
        else
        {
            retry = entity.retry_count;
        }
    } while (res != CURLE_OK && retry > 0);
    if (retry <= 0)
    {
        return_val = FAILED;
    }
    fclose(file);
    return return_val;
}

int patch_service::start(patch_entity &entity)
{
    int result = SUCCESS;
    int count = entity.retry_count;
    curl = curl_easy_init();
    while (count > 0)
    {
        result = download(entity);

        if (result == SUCCESS)
        {
            agent_utils::write_log("Download completed..", SUCCESS);
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(entity.retry_time_out));
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