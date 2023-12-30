#pragma once

#include "common.hpp"

#define MAX_RETRY_COUNT 3
#define TIME_OUT_ERROR 12

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
    download_props() : size(0L), maxSpeed(0), minSpeed(0), timeout(0), retry(MAX_RETRY_COUNT) {}
};

struct sftp_data
{
    string url;
    string username;
    string password;
};