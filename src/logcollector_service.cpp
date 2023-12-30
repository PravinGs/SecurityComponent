#include "service/logcollector_service.hpp"

bool log_service::parse_log_category(string &line, const vector<string> &log_levels)
{
    bool result = false;
    int max_level = 0;
    string network = "network";
    string ufw = "ufw";
    string lowercase_line = line;

    auto it = std::find(log_levels.begin(), log_levels.end(), "all");

    if (it != log_levels.end())
    {
        line += "|0";
        result = true;
    }
    else
    {
        for (string level : log_levels)
        {
            std::transform(lowercase_line.begin(), lowercase_line.end(), lowercase_line.begin(), [](unsigned char c)
                           { return std::tolower(c); });
            std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c)
                           { return std::tolower(c); });
            if (lowercase_line.find(level) != std::string::npos)
            {
                if (log_parser_level[agent_utils::to_lower_case(level)] > max_level)
                {
                    max_level = log_parser_level[agent_utils::to_lower_case(level)];
                    result = true;
                }
            }
            if (level == "none")
            {
                max_level = log_parser_level[agent_utils::to_lower_case(level)];
                result = true;
            }
        }

        line += "|" + std::to_string(max_level);
    }

    if (lowercase_line.find(network) != std::string::npos)
    {
        line += "|" + network;
    }
    else if (lowercase_line.find(ufw) != std::string::npos)
    {
        line += "|" + ufw;
    }
    else
    {
        line += "|sys";
    }

    return result;
}

bool filter_log(string &line, vector<string> log_levels)
{
    bool result = false;
    string lowercase_line = line;
    for (string level : log_levels)
    {
        std::transform(lowercase_line.begin(), lowercase_line.end(), lowercase_line.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        if (lowercase_line.find(level) != std::string::npos)
        {
            result = true;
        }
    }
    return result;
}

int log_service::read_syslog_file(log_entity &entity, vector<string> &logs)
{
    string standard_time_format_string, line;

    fstream file(entity.read_path, std::ios::in);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + entity.read_path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        agent_utils::convert_time_format(line.substr(0, 15), standard_time_format_string);     /* This func convert to standard time format */
        std::time_t current_time = agent_utils::string_to_time_t(standard_time_format_string); /* Convert string time to time_t format for comparision between time_t objects */

        if (current_time < entity.last_read_time)
        {
            continue;
        }

        string log, token;
        bool is_required = true;
        int index = 0;

        log += standard_time_format_string;
        std::stringstream stream(line.substr(16));

        while (std::getline(stream, token, entity.delimeter) && index < 3)
        {
            if (index == 2)
            {
                string message = token;
                while (std::getline(stream, token, entity.delimeter))
                {
                    message += entity.delimeter + token;
                }
                log += "|" + message;
                is_required = parse_log_category(log, entity.log_levels);
                index = 4;
                continue;
            }
            log += "|" + token;
            index++;
        }

        if (is_required)
        {
            logs.push_back(log);
            entity.count += 1;
        }

        std::time_t current_reading_time = agent_utils::string_to_time_t(entity.current_read_time);

        if (current_time > current_reading_time)
        {
            entity.current_read_time = standard_time_format_string;
        }

        if (current_time == entity.last_read_time)
        {
            entity.is_empty = false;
        }
    }

    file.close();

    if (entity.is_empty)
    {
        entity.read_path += ".1";
        read_syslog_file(entity, logs);
    }
    return SUCCESS;
}

int log_service::get_applog(log_entity &entity)
{
    int result = SUCCESS;
    vector<string> logs;
    agent_utils::write_log("Reading " + entity.name + " log starting...", DEBUG);
  
    if (entity.format == "syslog")
    {
        result = read_syslog_file(entity, logs);
    }
    else if (entity.format == "dpkg")
    {
        result = read_dpkg_logfile(entity, logs);
    }
    else if (entity.format == "applog")
    {
        result = read_applog_file(entity, logs);
    }
    else 
    {
        agent_utils::write_log("Currently this format not supported " + entity.format, WARNING);
        return WARNING;
    }

    if (entity.count > 0) { result = db.save(entity, logs); }

    else  { agent_utils::write_log("Read 0 logs for " + entity.name, WARNING); }


    return result;

}

int log_service::read_applog_file(log_entity& entity, vector<string> & logs)
{
    // string line;
    // bool is_critical_log = false;

    // fstream file(path);
    // if (!file)
    // {
    //     agent_utils::write_log(FILE_ERROR + path, FAILED);
    //     return FAILED;
    // }

    // while (std::getline(file, line))
    // {
    //     if (line.length() == 0)
    //     {
    //         continue;
    //     }

    //     string current_time_string = line.substr(0, 19);                               /* Extract the date time format fromt the line */
    //     std::time_t current_time = agent_utils::string_to_time_t(current_time_string); /* Convert string time to time_t format for comparision between time_t objects */
    //     if (current_time < last_written_time)
    //     {
    //         continue;
    //     }

    //     is_critical_log = filter_log(line, log_levels);
    //     if (is_critical_log)
    //     {
    //         logs.push_back(line);
    //     }
    //     std::time_t tempTime = agent_utils::string_to_time_t(next_reading_time);
    //     if (current_time > tempTime)
    //     {
    //         next_reading_time = current_time_string;
    //     }
    //     if (current_time == last_written_time)
    //     {
    //         flag = false;
    //     }
    // }
    // file.close();
    return SUCCESS;
}

int log_service::read_dpkg_logfile(log_entity &entity, vector<string> &logs)
{
    string standard_time_format_string, line;

    fstream file(entity.read_path, std::ios::in);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + entity.read_path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        string log, temp, host;
        agent_utils::get_hostname(host);
        string current_time_string = line.substr(0, 19);
        std::time_t current_time = agent_utils::string_to_time_t(current_time_string); /* Convert string time to time_t format for comparision between time_t objects */
        if (current_time < entity.last_read_time)
        {
            continue;
        }

        log += current_time_string;
        log += "|" + host;
        log += "| dpkg";
        temp = line.substr(20);
        log += "|" + temp;
        entity.count += 1;
        logs.push_back(log);


        std::time_t current_reading_time = agent_utils::string_to_time_t(entity.current_read_time);
        if (current_time > current_reading_time)
        {
            entity.current_read_time = current_time_string;
        }
        if (current_time == entity.last_read_time)
        {
            entity.is_empty = false;
        }
    }
    if (entity.is_empty)
    {
        entity.read_path += ".1";
        read_dpkg_logfile(entity, logs);
    }
    return SUCCESS;
}

int log_service::read_remote_syslog(UdpQueue &queue, vector<string> &logs)
{
    return queue.getMessage(logs);
}

int log_service::get_syslog(log_entity &entity)
{
    int result = SUCCESS;
    vector<string> logs;

    if (entity.name == "syslog" || entity.name == "auth")
    {
        if (entity.remote == 'y' || entity.remote == 'Y')
        {
            UdpQueue queue;
            result = read_remote_syslog(queue, logs);
            queue.stop();
        }
        else
        {
            result = read_syslog_file(entity, logs);
        }
    }
    else if (entity.name == "dpkg")
    {
        result = read_dpkg_logfile(entity, logs);
    }
    
    if (entity.count > 0)
    {
        result = db.save(entity, logs);
    }
    else
    {
        agent_utils::write_log("Read 0 logs for " + entity.name, DEBUG);
    }

    return result;
}

log_service::~log_service() {}