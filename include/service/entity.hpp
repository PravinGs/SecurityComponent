#pragma once 
typedef struct log_entity log_entity;

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
