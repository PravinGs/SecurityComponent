#pragma once 

typedef struct log_entity log_entity;
typedef struct analysis_entity analysis_entity;
typedef struct process_entity process_entity;
typedef struct patch_entity patch_entity;

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

    log_entity(): count(0), is_empty(true) {}
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
    string application;
    string application_root_path;
    int max_download_speed;
    int min_download_speed;
    int retry_time_out;
};