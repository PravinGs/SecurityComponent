#include "common.hpp"

int os::current_day = 0;
int os::current_month = 0;
int os::current_year = 0;
string os::host_name = "";
bool agent_utils::syslog_enabled = true;
fstream agent_utils::logfp;
std::mutex logfile_mutex;
bool agent_utils::debug = false;

void agent_utils::setup_logger()
{
    fstream file("/var/log/agent.log", std::ios::in | std::ios::binary);
    long size = 0L;
    if (!file.is_open())
    {
        file.seekg(0, std::ios::end);
        size = file.tellg();
        syslog(LOG_INFO, "Syslog testing");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file.seekg(0, std::ios::end);
        long updated_size = file.tellg();
        if (updated_size == size || updated_size == 0L)
        {
            agent_utils::syslog_enabled = false;
        }
    }
    else
    {
        agent_utils::syslog_enabled = false;
    }
}

void agent_utils::print_next_execution_time(std::tm *next_time_info)
{
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", next_time_info);
    std::string next_time_str(buffer);
    agent_utils::write_log("agent_utils: print_next_execution_time: next execution time: " + next_time_str, DEBUG);
}

void agent_utils::print_duration(const std::chrono::duration<double> &duration)
{
    int hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    auto remaining_duration = duration - std::chrono::hours(hours);
    int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining_duration).count();
    remaining_duration -= std::chrono::minutes(minutes);
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining_duration).count();
    agent_utils::write_log("agent_utils: print_duration: duration until next execution: " + std::to_string(hours) + " hours, " + std::to_string(minutes) + " minutes, " + std::to_string(seconds) + " seconds.", DEBUG);
}

string agent_utils::trim(string line)
{
    const auto str_begin = line.find_first_not_of(" \t");
    if (str_begin == string::npos)
        return "";

    const auto strEnd = line.find_last_not_of(" \t");
    const auto strRange = strEnd - str_begin + 1;

    string str = line.substr(str_begin, strRange);
    return (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') ? str.substr(1, str.length() - 2) : str;
}

void agent_utils::update_log_written_time(const string &app_name, const string &time)
{
    string file_path = BASE_CONFIG_DIR;
    if (!os::is_dir_exist(file_path))
    {
        os::create_dir(file_path);
    }
    file_path += BASE_CONFIG_TMP;
    if (!os::is_dir_exist(file_path))
    {
        os::create_dir(file_path);
    }
    file_path += app_name;
    fstream file(file_path, std::ios::out);
    if (!file.is_open())
    {
        write_log("agent_utils: update_log_written_time: " + INVALID_FILE + file_path, FAILED);
        return;
    }
    file << time;
    write_log("agent_utils: update_log_written_time: time " + time + " updated to " + file_path, SUCCESS);
    file.close();
    return;
}

string agent_utils::to_lower_case(string &str)
{
    string lower_case_string;
    for (char ch : str)
    {
        lower_case_string += std::tolower(ch);
    }
    return lower_case_string;
}

int agent_utils::get_hostname(string &host)
{
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        hostname[strlen(hostname)] = '\0';
        host = hostname;
    }
    else
    {
        write_log("agent_utils: get_hostname: failed to get the hostname.", FAILED);
        return FAILED;
    }
    return SUCCESS;
}

string agent_utils::get_current_time()
{
    char current_time[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(current_time, sizeof(current_time), "%Y-%m-%d %H:%M:%S", t);
    // string s_time(current_time);
    // auto mid = s_time.find_first_of(' ');
    // string time = s_time.substr(0, mid) + "_" + s_time.substr(mid + 1);
    return current_time;
}

bool agent_utils::is_valid_time_string(const std::string &timeString)
{
    std::tm tm_struct = {};
    return (strptime(timeString.c_str(), "%b %e %H:%M:%S", &tm_struct) != nullptr) ? true : false;
}

int agent_utils::convert_time_format(const std::string &inputTime, std::string &formatTime)
{
    if (!is_valid_time_string(inputTime))
    {
        return FAILED;
    }
    std::stringstream ss(inputTime);
    string year, month, day, time;
    std::tm tm = {};
    int m = 0, d;
    month = trim(inputTime.substr(0, 4));
    for (; m < (int)MONTHS.size(); m++)
    {
        if (MONTHS[m] == month)
        {
            m++;
            break;
        }
    }
    month = "";
    if (m <= 9 && m > 0)
    {
        month = "0";
    }
    month += std::to_string(m);
    day = trim(inputTime.substr(4, 6));
    d = std::stoi(day);
    day = "";
    if (d <= 9 && d > 0)
    {
        day = "0";
    }
    day += std::to_string(d);
    time = trim(inputTime.substr(6, 15));
    std::time_t current_time = std::time(nullptr);
    std::tm *currentTm = std::localtime(&current_time);
    tm.tm_year = currentTm->tm_year;
    tm.tm_year += 1900;

    formatTime = std::to_string(tm.tm_year) + "-" + month + "-" + day + " " + time;
    return SUCCESS;
}

void agent_utils::write_log(const string &log)
{
    string time = get_current_time();
    string line = time + " " + log + "\n";
    std::lock_guard<std::mutex> lm(logfile_mutex);
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.write(line.c_str(), line.size());
    }
    agent_utils::backup_log_file();
    return;
}

void agent_utils::write_log(const string &log, int logLevel)
{
#if NO_DEBUG && logLevel == DEBUG
    return;
#endif

    string time = get_current_time();
    string line;
    switch (logLevel)
    {
    case SUCCESS:
        line = time + " : [SUCCESS] " + log;
        break;
    case INFO:
        line = time + " : [INFO] " + log;
        break;
    case FAILED:
        line = time + " : [ERROR] " + log;
        break;
    case WARNING:
        line = time + " : [WARNING] " + log;
        break;
    case CRITICAL:
        line = time + " : [CRITICAL] " + log;
        break;
    case DEBUG:
        line = time + " : [DEBUG] " + log;
    default:
        break;
    }
    line += "\n";
    std::lock_guard<std::mutex> lm(logfile_mutex);
    {
        if (syslog_enabled)
        {
            syslog(LOG_INFO, " %s", line.c_str());
        }
        else
        {
            if (!agent_utils::logfp.is_open())
            {
                agent_utils::logfp.open(LOG_PATH, std::ios::app);
            }
            agent_utils::logfp.write(line.c_str(), line.size());
            agent_utils::logfp.flush();
        }
    }
    agent_utils::backup_log_file();
    return;
}

void agent_utils::backup_log_file()
{
    std::streampos size;
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.seekg(0, std::ios::end);
        size = agent_utils::logfp.tellg();
    }
    if (size > 5120)
    {
        os::compress_file(LOG_PATH);
        agent_utils::logfp.close();
        std::filesystem::remove(LOG_PATH);
        agent_utils::logfp.open(LOG_PATH, std::ios::app);
    }
}

int os::compress_file(const string &log_file)
{
    int result = SUCCESS;
    string line;
    string current_file = log_file;
    if (log_file == LOG_PATH)
    {
        vector<string> files;
        os::get_regular_files(BASE_LOG_DIR, files);
        current_file += std::to_string(files.size());
    }

    if (current_file.size() == 0)
    {
        agent_utils::write_log("agent_utils: compress_file: no global log file updated in the code for os", FAILED);
        return FAILED;
    }
    fstream file(log_file, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        agent_utils::write_log("agent_utils: compress_file: no file exist for backup ( " + log_file + " )", FAILED);
        return FAILED;
    }
    gzFile zLog;
    string zipFile = current_file + ".gz";
    zLog = gzopen(zipFile.c_str(), "w");
    if (!zLog)
    {
        agent_utils::write_log("agent_utils: compress_file: " + FCREATION_FAILED + zipFile, FAILED);
        file.close();
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.size() == 0)
            continue;
        if (gzwrite(zLog, line.c_str(), static_cast<unsigned int>(line.size())) != (int)line.size())
        {
            agent_utils::write_log("agent_utils: compress_file: " + FWRITE_FAILED + zipFile, FAILED);
            result = FAILED;
            break;
        }
    }
    file.close();
    gzclose(zLog);
    if (result == SUCCESS)
    {
        os::delete_file(current_file);
    }
    else
    {
        agent_utils::write_log("agent_utils: compress_file: " + FDELETE_FAILED + current_file, FAILED);
    }
    return result;
}

string os::sign(const string &file, const string &sign_key)
{
    if (!os::is_file_exist(file))
    {
        agent_utils::write_log("os: sign: file not exist: " + file, FAILED);
        return "";
    }

    std::ifstream file_data(file, std::ios::binary);
    if (!file_data.is_open())
    {
        agent_utils::write_log("os: sign: unable to open file: " + file, FAILED);
        return "";
    }
    string data((std::istreambuf_iterator<char>(file_data)), std::istreambuf_iterator<char>());

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length;

    HMAC_CTX *hmac_ctx = HMAC_CTX_new();
    HMAC_Init_ex(hmac_ctx, sign_key.c_str(), sign_key.length(), EVP_sha256(), nullptr);
    HMAC_Update(hmac_ctx, reinterpret_cast<const unsigned char *>(data.c_str()), data.length());
    HMAC_Final(hmac_ctx, hash, &hash_length);
    HMAC_CTX_free(hmac_ctx);
    
    return string(reinterpret_cast<char *>(hash), hash_length);
}

bool os::verify_signature(const string &file, const string &sign_key, const string &signed_data)
{
    string hash = sign(file, sign_key);

    return (hash == signed_data) ? true : false;
}

bool os::is_file_exist(const string &file)
{
    fstream fp(file, std::ios::in | std::ios::binary);
    if (!fp.is_open())
    {
        agent_utils::write_log(FILE_ERROR + file, CRITICAL);
        return false;
    }
    fp.close();
    return true;
}

string os::get_path_or_backup_file_path(string filename)
{
    string non_empty_path;
    if (std::filesystem::exists(filename))
    {
        std::ifstream file(filename, std::ios::binary);
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();

        if (fileSize == 0)
        {
            non_empty_path = filename + ".1";
        }
        else
        {
            non_empty_path = filename;
        }
        file.close();
    }
    else
    {
        non_empty_path = "";
    }
    return non_empty_path;
}

string os::get_file_by_current_day(const string &app_name)
{
    string current_log_file = BASE_LOG_DIR;
    current_log_file += BASE_LOG_ARCHIVE;
    current_log_file += std::to_string(current_year) + "/" + MONTHS[current_month - 1] + "/" + std::to_string(current_day) + "-" + app_name;
    return current_log_file;
}

int os::handle_local_log_file(int day, int month, int year, string &file_path, const string &app_name)
{
    int result;
    string current_dir = BASE_LOG_DIR;
    if (day != current_day)
    {
        string current_log_file = get_file_by_current_day(app_name);

        if ((result = compress_file(current_log_file)) == FAILED)
        {
            return FAILED;
        }
        result = create_log_file(day, month, year, file_path, app_name);
    }
    else
    {
        result = create_log_file(day, month, year, file_path, app_name);
    }

    return result;
}

int os::create_log_file(int curr_day, int curr_month, int curr_year, string &file_path, const string &app_name)
{
    string current_dir = BASE_LOG_DIR;
    if (!is_dir_exist(current_dir))
    {
        agent_utils::write_log("os: create_log_file: " + INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log("os: create_log_file: " + NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += BASE_LOG_ARCHIVE;
    if (!is_dir_exist(current_dir))
    {
        agent_utils::write_log("os: create_log_file: " + INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log("os: create_log_file: " + NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + std::to_string(curr_year);
    if (!is_dir_exist(current_dir))
    {
        agent_utils::write_log("os: create_log_file: " + INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log("os: create_log_file: " + NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + MONTHS[curr_month - 1];
    if (!is_dir_exist(current_dir))
    {
        agent_utils::write_log("os: create_log_file: " + INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log("os: create_log_file: " + NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + std::to_string(curr_day) + "-" + app_name;
    file_path = current_dir;
    if (std::filesystem::exists(current_dir))
    {
        return SUCCESS;
    }
    fstream new_file(file_path, std::ios::out);
    if (new_file)
    {
        new_file.close();
        current_day = curr_day;
        current_month = curr_month;
        current_year = curr_year;
        return SUCCESS;
    }
    else
    {
        agent_utils::write_log("os: create_log_file: " + FCREATION_FAILED + file_path, FAILED);
    }
    return FAILED;
}

int os::delete_file(const string &fileName)
{
    if (std::filesystem::exists(fileName))
    {
        try
        {
            if (std::filesystem::remove(fileName))
            {
                return SUCCESS;
            }
        }
        catch (const std::exception &e)
        {
            string error(e.what());
            agent_utils::write_log("os: delete_file: " + error, FAILED);
        }
    }
    return FAILED;
}

int os::create_dir(const string& dir_name)
{
    if (std::filesystem::exists(dir_name))
    {
        return SUCCESS;
    }
    else
    {
        try
        {
            if (std::filesystem::create_directory(dir_name))
                return SUCCESS;
        }
        catch (const std::exception &e)
        {
            string error(e.what());
            agent_utils::write_log("os: create_dir: " + error, FAILED);
        }
    }
    return FAILED;
}

int os::create_file(const string &file_path)
{
    try
    {
        if (is_file_exist(file_path))
        {
            return SUCCESS;
        }
        std::ofstream file(file_path);
        if (file.is_open())
        {
            agent_utils::write_log("os: create_file: new file creation: " + file_path, DEBUG);
            file.close();
            return SUCCESS;
        }
    }
    catch (const std::exception &e)
    {
        string error(e.what());
        agent_utils::write_log("os: create_file: " + error, FAILED);
    }
    return FAILED;
}

bool os::is_dir_exist(const string &dir_name)
{
    return std::filesystem::exists(dir_name);
}

int os::get_regular_files(const string &directory, vector<string> &files)
{
    int result = SUCCESS;
    try
    {
        string parent = directory;
        for (const auto &entry : std::filesystem::directory_iterator(directory))
        {
            if (std::filesystem::is_regular_file(entry.path()))
            {
                string child = entry.path();
                files.push_back(child);
            }
        }
    }
    catch (exception &e)
    {
        result = FAILED;
        string except = e.what();
        agent_utils::write_log("os: get_regular_files: " + except, FAILED);
    }
    return result;
}

std::time_t agent_utils::string_to_time_t(const string &datetime)
{
    const char *STANDARD_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, STANDARD_TIME_FORMAT);
    return std::mktime(&tm);
}

string os::get_json_file_path()
{
    string time = agent_utils::get_current_time();
    auto mid = time.find_first_of(' ');
    return time.substr(0, mid) + "_" + time.substr(mid+1);
}

string os::get_json_write_path(const string &type)
{
    string json_file_path = os::get_json_file_path();
    string file_path = BASE_LOG_DIR;

    if (!os::is_dir_exist(file_path))
    {
        os::create_dir(file_path);
    }
    file_path += "json";
    if (!os::is_dir_exist(file_path))
    {
        os::create_dir(file_path);
    }
    file_path += "/" + type;
    if (!os::is_dir_exist(file_path))
    {
        os::create_dir(file_path);
    }
    file_path += "/" + json_file_path + ".json";
    std::ofstream file(file_path);
    if (!file)
    {
        agent_utils::write_log("os: get_json_write_path: " + FILE_ERROR + file_path, FAILED);
        return "";
    }
    file.close();
    return file_path;
}