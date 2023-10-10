#include "service/logcollector_service.hpp"

string to_lower_case(string &str)
{
    string lower_case_string;
    for (char ch : str)
    {
        lower_case_string += std::tolower(ch);
    }
    return lower_case_string;
}

int log_service::save_json(Json::Value &json, const string &path, const vector<string> &logs, const vector<string> &columns, const char &delimeter)
{
    string json_write_path = path;
    if (get_json_write_path(json_write_path) == FAILED) return FAILED;

    fstream file(json_write_path, std::ios::out);
    Json::StreamWriterBuilder writer_builder;
    if (!file)
    {
        agent_utils::write_log(FWRITE_FAILED + json_write_path, FAILED);
        return FAILED;
    }

    json["LogObjects"] = Json::Value(Json::arrayValue);
    for (auto log : logs)
    {
        Json::Value jsonLog;

        standard_log_attrs fLog = standard_log_attrs(log);
        jsonLog["TimeGenerated"] = fLog.timestamp;
        jsonLog["UserLoginId"] = fLog.user;
        jsonLog["ServiceName"] = fLog.program;
        jsonLog["Message"] = fLog.message;
        jsonLog["LogLevel"] = fLog.level;
        jsonLog["LogCategory"] = fLog.category;

        json["LogObjects"].append(jsonLog);
    }
    std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
    writer->write(json, &file);
    file.close();
    agent_utils::write_log(FWRITE_SUCCESS + json_write_path, SUCCESS);
    return SUCCESS;
}

void log_service::categorize(string &line, const vector<string> &log_levels)
{
    int max_level = 0;
    string network = "network";
    string ufw = "ufw";
    string lowercase_line = line;
    for (string level : log_levels)
    {
        std::transform(lowercase_line.begin(), lowercase_line.end(), lowercase_line.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        if (lowercase_line.find(level) != std::string::npos)
        {
            if (_logLevel[to_lower_case(level)] > max_level)
            {
                max_level = _logLevel[to_lower_case(level)];
            }
        }
    }

    line += "|" + std::to_string(max_level);

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

    return;
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

int log_service::read_syslog_file(const string &path, vector<string> &logs, const char &delimeter, const string &last_read_time, bool &flag, const vector<string> &log_levels, string &next_reading_time)
{
    const string sep = "|";
    string formatted_time, line;
    std::time_t last_written_time = agent_utils::format_string_time(last_read_time);

    fstream file(path, std::ios::in);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        string currentTime = line.substr(0, 15);                         /* Extract the date time format fromt the line */
        agent_utils::convert_time_format(currentTime, formatted_time);       /* This func convert to standard time format */
        std::time_t cTime = agent_utils::format_string_time(formatted_time); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < last_written_time)
        {
            continue;
        }
        string log, token;
        log += formatted_time;
        std::stringstream stream(line.substr(16));
        int index = 0;
        while (std::getline(stream, token, delimeter) && index < 3)
        {
            if (index == 2)
            {
                string message = token;
                while (std::getline(stream, token, ' '))
                {
                    message += ' ' + token;
                }
                log += sep + message;
                categorize(log, log_levels);
                index = 4;
                continue;
            }
            log += sep + token;
            index++;
        }

        logs.push_back(log);
        std::time_t tempTime = agent_utils::format_string_time(next_reading_time);
        if (cTime > tempTime)
        {
            next_reading_time = formatted_time;
        }
        if (cTime == last_written_time)
        {
            flag = false;
        }
    }

    file.close();

    if (flag)
    {
        read_syslog_file(path + ".1", logs, delimeter, last_read_time, flag, log_levels, next_reading_time);
    }
    return SUCCESS;
}

int log_service::get_syslog(const string &app_name, Json::Value &json, const vector<string> &log_attributes, const string &path, string &last_read_time, const vector<string> &log_levels, const char &remote)
{
    bool flag = true;
    int result = SUCCESS;
    const char delimeter = ' ';
    vector<string> logs;

    string log_dir = BASE_LOG_DIR;
    log_dir += BASE_LOG_ARCHIVE;
    string next_reading_time = last_read_time;
    

    if (strcmp(app_name.c_str(), "syslog") == 0 || strcmp(app_name.c_str(), "auth") == 0)
    {

        if (remote == 'y' || remote == 'Y')
        {
            UdpQueue queue;
            result = read_remote_syslog(queue, logs);
            queue.stop();
        }
        else
        {
            result = read_syslog_file(path, logs, delimeter, last_read_time, flag, log_levels, next_reading_time);
        }
        if (logs.size() == 0 || result == FAILED)
        {
            agent_utils::write_log("Read 0 logs for" + app_name);
        }
        else
        {
            last_read_time = next_reading_time;
            agent_utils::write_log(app_name + " logs collected", INFO);
            if (_config_service.to_vector(logs[0], '|').size() < log_attributes.size())
            {
                agent_utils::write_log("Invalid Log Attributes configured for " + app_name, FAILED);
                result = FAILED;
            }
            else
            {
                string fileName = next_reading_time + "-" + app_name;
                result = save_json(json, fileName, logs, log_attributes, '|');
            }

            agent_utils::write_log("Storing " + app_name + " logs started", INFO);
            if ((result = save_read_logs(logs, app_name)) == SUCCESS)
            {

                agent_utils::write_log(FWRITE_SUCCESS + log_dir, INFO);
            }
            else
            {
                agent_utils::write_log(FWRITE_FAILED + log_dir, FAILED);
            }
        }
    }
    else if (strcmp(app_name.c_str(), "dpkg") == 0)
    {
        result = read_dpkg_logfile(path, logs, last_read_time, next_reading_time, flag);
        last_read_time = next_reading_time;
        if (logs.size() == 0)
        {
            agent_utils::write_log("Read 0 logs for " + app_name);
        }
        else
        {
            agent_utils::write_log("Storing " + app_name + " logs started", INFO);
            if ((result = save_read_logs(logs, app_name)) == SUCCESS)
            {
                agent_utils::write_log(FWRITE_SUCCESS + log_dir, INFO);
            }
            else
            {
                agent_utils::write_log(FWRITE_FAILED + log_dir, FAILED);
            }
        }
    }

    return result;
}

int log_service::get_applog(Json::Value &json, const vector<string> &log_attributes, const string &readDir, const string &writePath, string &last_read_time, const vector<string> &log_levels, const char &delimeter)
{
    vector<string> logs;
    bool flag = true;
    vector<std::filesystem::directory_entry> files = get_dir_files(readDir);
    int index = (int)files.size() - 1;
    string next_reading_time = last_read_time;
    while (flag && index >= 0)
    {
        string path = files[index].path();
        if (!std::filesystem::is_regular_file(path))
        {
            index--;
            continue;
        }

        if (read_applog_file(path, logs, delimeter, last_read_time, flag, log_levels, next_reading_time) == FAILED || logs.size() == 0)
        {
            return FAILED;
        }
        agent_utils::write_log("Applog collected from " + path, INFO);
        index--;
    }

    if (_config_service.to_vector(logs[0], delimeter).size() < log_attributes.size())
    {
        agent_utils::write_log("Invalid Log Attributes configured for Applog", FAILED);
        return FAILED;
    }
    last_read_time = next_reading_time;
    return save_json(json, writePath, logs, log_attributes, delimeter);
}

int log_service::read_applog_file(const string &path, vector<string> &logs, const char &delimeter, const string &last_read_time, bool &flag, const vector<string> &log_levels, string &next_reading_time)
{
    fstream file(path);
    string line;
    std::time_t last_written_time = agent_utils::format_string_time(last_read_time);
    bool is_critical_log = false;
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.length() == 0)
        {
            continue;
        }

        string currentTime = line.substr(0, 19);                       /* Extract the date time format fromt the line */
        std::time_t cTime = agent_utils::format_string_time(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < last_written_time)
        {
            continue;
        }

        is_critical_log = filter_log(line, log_levels);
        if (is_critical_log)
        {
            logs.push_back(line);
        }
        std::time_t tempTime = agent_utils::format_string_time(next_reading_time);
        if (cTime > tempTime)
        {
            next_reading_time = currentTime;
        }
        if (cTime == last_written_time)
        {
            flag = false;
        }
    }
    file.close();
    return SUCCESS;
}

vector<std::filesystem::directory_entry> log_service::get_dir_files(const string &directory)
{
    vector<std::filesystem::directory_entry> files;

    for (const auto &file : std::filesystem::directory_iterator(directory))
    {
        files.push_back(file);
    }

    std::sort(files.begin(), files.end(), [](const std::filesystem::directory_entry &a, const std::filesystem::directory_entry &b)
              { return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b); });

    return files;
}

int log_service::save_read_logs(const vector<string> &logs, const string &app_name)
{
    string file_path;
    auto today = std::chrono::system_clock::now();
    auto time_info = std::chrono::system_clock::to_time_t(today);
    std::tm *tm_info = std::localtime(&time_info);

    int day = tm_info->tm_mday;
    int month = tm_info->tm_mon + 1;
    int year = tm_info->tm_year + 1900;

    if (os::handle_local_log_file(day, month, year, file_path, app_name) == FAILED)
    {
        return FAILED;
    }

    fstream file(file_path, std::ios::app);

    for (string line : logs)
    {
        file << line << "\n";
    }

    file.close();

    return SUCCESS;
}

int log_service::get_json_write_path(string &timestamp)
{
    string file_path = BASE_LOG_DIR;

    if (os::is_dir_exist(file_path) == FAILED)
    {
        os::create_dir(file_path);
    }

    file_path += "json";

    if (os::is_dir_exist(file_path) == FAILED)
    {
        os::create_dir(file_path);
    }

    file_path += "/" + timestamp + ".json";

    std::ofstream file(file_path);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + file_path, FAILED);
        return FAILED;
    }
    file.close();
    timestamp = file_path;
    return SUCCESS;
}

int log_service::read_dpkg_logfile(const string &path, vector<string> &logs, string &last_read_time, string &next_reading_time, bool &flag)
{
    string formatted_time, line;
    const string format = "dpkg";
    std::time_t last_written_time = agent_utils::format_string_time(last_read_time);

    fstream file(path, std::ios::in);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        string log, temp, host;
        agent_utils::get_hostname(host);
        string currentTime = line.substr(0, 19);
        std::time_t cTime = agent_utils::format_string_time(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < last_written_time)
        {
            continue;
        }

        log += currentTime;
        log += "|"+host;
        log += "|" + format;
        temp = line.substr(20);
        log += "|"+temp;
        logs.push_back(log);

        std::time_t tempTime = agent_utils::format_string_time(next_reading_time);
        if (cTime > tempTime)
        {
            next_reading_time = currentTime;
        }
        if (cTime == last_written_time)
        {
            flag = false;
        }
    }
    if (flag)
    {
        read_dpkg_logfile(path + ".1", logs, last_read_time, next_reading_time, flag);
    }
    return SUCCESS;
}

int log_service::read_remote_syslog(UdpQueue &queue, vector<string> &logs)
{
    return queue.getMessage(logs);
}

log_service::~log_service() {}