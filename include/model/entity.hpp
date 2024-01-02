#pragma once
#include "common.hpp"

typedef struct log_entity log_entity;
typedef struct analysis_entity analysis_entity;
typedef struct process_entity process_entity;
typedef struct patch_entity patch_entity;
typedef struct mqtt_entity mqtt_entity;
typedef struct rest_entity rest_entity;

struct log_entity
{
    int count;
    string format;
    string name;
    string read_path;
    char delimeter;
    string write_path;
    vector<string> json_attributes;
    string columns;
    string time_pattern;
    string storage_type;
    vector<string> log_levels;
    std::time_t last_read_time;
    string current_read_time;
    char remote;
    bool is_empty;

    log_entity() : count(0), is_empty(true) {}
};

struct analysis_entity
{
    string log_path;
    string decoder_path;
    string rules_path;
    string write_path;
    string time_pattern;
    string storage_type;
};

struct process_entity
{
    string write_path;
    string time_pattern;
    string storage_type;
};

struct patch_entity
{
    long size;
    int max_download_speed;
    int min_download_speed;
    int retry_time_out;
    int retry_count;
    bool is_sftp;
    bool is_secure;
    string ca_cert_path;
    string client_cert_path;
    string application;
    string application_root_path;
    string download_path;
    string username;
    string password;
    string url;

    patch_entity() : size(0L), max_download_speed(0), min_download_speed(0), retry_time_out(0), retry_count(0), is_sftp(true), is_secure(true) {}
};

struct mqtt_entity
{
    int port; // local dev testing only.
    int qos;
    bool is_secure;
    string client_id;
    string conn_string;
    vector<string> topics; // this type could be key:value (or) vector (or) single char[]
    string ca_cert_path;
    string client_cert_path;

    mqtt_entity() : port(8000), qos(1), is_secure(true) {}
};

struct rest_entity
{
    string logs_post_url;
    string ids_post_url;
    string patch_get_url;
    string resources_post_url;
    string ca_cert_path;
    string client_cert_path;
};