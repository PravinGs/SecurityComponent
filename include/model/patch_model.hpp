#pragma once

#include "entity.hpp"

#define PATCH_DEFAULT_RETRY_COUNT 3
#define PATCH_DEFAULT_RETRY_TIMEOUT 30
#define PATCH_DEFAULT_MAX_DOWNLOAD_SPEED 10240
#define PATCH_DEFAULT_MIN_DOWNLOAD_SPEED 0
#define TIME_OUT_ERROR 12

const string PATCH_FILE_DOWNLOAD_PATH = "/etc/scl/tmp/";

typedef struct download_props download_props;

struct download_props
{
    long size;
    int maxSpeed;
    int minSpeed;
    int timeout;
    int retry;
    string writePath;
    string rootDir;
    string url;
    string fileName;
    string downloadPath;
    download_props() : size(0L), maxSpeed(0), minSpeed(0), timeout(0), retry(0) {}
};

struct sftp_data
{
    string url;
    string username;
    string password;
};