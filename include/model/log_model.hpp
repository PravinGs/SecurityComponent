#pragma once

#include "common.hpp"

typedef struct standard_log_attrs standard_log_attrs;

static map<string, int> LogCategory{{"sys", 2}, {"network", 3}, {"ufw", 4}};

struct standard_log_attrs
{
    string timestamp;
    string user;
    string program;
    string message;
    int level;
    int category;

    standard_log_attrs(const string &log)
    {
        string t_level, t_category;
        std::stringstream ss(log);
        std::getline(ss, timestamp, '|');
        std::getline(ss, user, '|');
        std::getline(ss, program, '|');
        std::getline(ss, message, '|');
        std::getline(ss, t_level, '|');
        std::getline(ss, t_category, '|');
        level = handle_exception(t_level);
        category = LogCategory[t_category];
    }

    int handle_exception(const string &level)
    {
        int r;
        try
        {
            r = std::stoi(level);
        }
        catch (const std::exception &e)
        {
            r = 0;
        }
        return r;
    }

    ~standard_log_attrs() {}
};