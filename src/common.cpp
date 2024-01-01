#include "common.hpp"

int os::current_day = 0;
int os::current_month = 0;
int os::current_year = 0;
bool agent_utils::syslog_enabled = true;
fstream agent_utils::logfp;
std::mutex logMutex;
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
        long updateSize = file.tellg();
        if (updateSize == size || updateSize == 0L)
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
    agent_utils::write_log("Next execution time: " + next_time_str, DEBUG);
}

void agent_utils::print_duration(const std::chrono::duration<double> &duration)
{
    int hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    auto remaining_duration = duration - std::chrono::hours(hours);
    int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining_duration).count();
    remaining_duration -= std::chrono::minutes(minutes);
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining_duration).count();
    agent_utils::write_log("Duration until next execution: " + std::to_string(hours) + " hours, " + std::to_string(minutes) + " minutes, " + std::to_string(seconds) + " seconds.", DEBUG);
}

string agent_utils::trim(string line)
{
    const auto strBegin = line.find_first_not_of(" \t");
    if (strBegin == string::npos)
        return "";

    const auto strEnd = line.find_last_not_of(" \t");
    const auto strRange = strEnd - strBegin + 1;

    string str = line.substr(strBegin, strRange);
    return (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') ? str.substr(1, str.length() - 2) : str;
}

void agent_utils::update_log_written_time(const string& app_name, const string& time)
{
    string filePath = BASE_CONFIG_DIR;
    if (os::is_dir_exist(filePath) == FAILED)
    {
        os::create_dir(filePath);
    }
    filePath += BASE_CONFIG_TMP;
    if (os::is_dir_exist(filePath) == FAILED)
    {
        os::create_dir(filePath);
    }
    filePath += app_name;
    fstream file(filePath, std::ios::out);
    if (!file.is_open())
    {
        write_log(INVALID_FILE + filePath, FAILED);
        return;
    }
    file << time;
    write_log("Time " + time + " updated to " + filePath, SUCCESS);
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
        write_log("Failed to get the hostname.", FAILED);
        return FAILED;
    }
    return SUCCESS;
}

string agent_utils::get_current_time()
{
    char currentTime[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", t);
    return currentTime;
}

bool agent_utils::is_valid_time_string(const std::string& timeString) {
    std::tm tm_struct = {};
    return (strptime(timeString.c_str(), "%b %e %H:%M:%S", &tm_struct) != nullptr) ? true : false;
}

int agent_utils::convert_time_format(const std::string &inputTime, std::string &formatTime)
{
    if (!is_valid_time_string(inputTime)) { return FAILED; }
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
    // Get the current year
    std::time_t currentTime = std::time(nullptr);
    std::tm *currentTm = std::localtime(&currentTime);
    tm.tm_year = currentTm->tm_year; // Update the year in tm
    tm.tm_year += 1900;              // Add 1900 to get the correct year value

    formatTime = std::to_string(tm.tm_year) + "-" + month + "-" + day + " " + time;
    return SUCCESS;
}

void agent_utils::write_log(const string& log)
{
    string time = get_current_time();
    string line = time + " " + log + "\n";
    std::lock_guard<std::mutex> lm(logMutex);
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.write(line.c_str(), line.size());
    }
    agent_utils::backup_log_file();
    return;
}

void agent_utils::write_log(const string& log, int logLevel)
{
    #if NO_DEBUG && logLevel == DEBUG
        return;
    #endif

    string time = get_current_time();
    string line;
    switch (logLevel)
    {
    case SUCCESS:
        line =  time + " : [SUCCESS] " + log;
        break;
    case INFO:
        line =  time + " : [INFO] " + log;
        break;
    case FAILED:
        line =  time + " : [ERROR] " + log;
        break;
    case WARNING:
        line =  time + " : [WARNING] " + log;
        break;
    case CRITICAL:
        line =  time + " : [CRITICAL] " + log;
        break;
    case DEBUG:
        line =  time + " : [DEBUG] " + log;
    default:
        break;
    }
    line += "\n";
    std::lock_guard<std::mutex> lm(logMutex);
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
    if (agent_utils::logfp.is_open()){
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

int os::compress_file(const string& log_file)
{
    int result = SUCCESS;
    string line;
    string current_file = log_file;
    if (log_file == LOG_PATH){
        vector<string> files;
        os::get_regular_files(BASE_LOG_DIR, files);
        current_file += std::to_string(files.size());
    }
    
    if (current_file.size() == 0)
    {
        agent_utils::write_log("No global log file updated in the code for os", FAILED);
        return FAILED;
    }
    fstream file(log_file, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        agent_utils::write_log("No file exist for backup ( " + log_file + " )", FAILED);
        return FAILED;
    }
    gzFile zLog;
    string zipFile = current_file + ".gz";
    zLog = gzopen(zipFile.c_str(), "w");
    if (!zLog)
    {
        agent_utils::write_log(FCREATION_FAILED + zipFile, FAILED);
        file.close();
        return FAILED;
    }
    
    while (std::getline(file, line))
    {
        if (line.size() == 0)
            continue;
        if (gzwrite(zLog, line.c_str(), static_cast<unsigned int>(line.size())) != (int)line.size())
        {
            agent_utils::write_log(FWRITE_FAILED + zipFile, FAILED);
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
        agent_utils::write_log(FDELETE_FAILED + current_file, FAILED);
    }
    return result;
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

string os::get_file_by_current_day(const string& app_name)
{
    string current_log_file = BASE_LOG_DIR;
    current_log_file       += BASE_LOG_ARCHIVE;
    current_log_file += std::to_string(current_year) + "/" + MONTHS[current_month-1] + "/" + std::to_string(current_day) + "-" + app_name;
    return current_log_file;
}

int os::handle_local_log_file(int day, int month, int year, string& filePath, const string& app_name)
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
        result = create_log_file(day, month, year, filePath, app_name);
    }
    else
    {
        result = create_log_file(day, month, year, filePath, app_name);
    }

    return result;
}

int os::create_log_file(int cDay, int cMonth, int cYear, string& filePath, const string& app_name)
{
    string current_dir = BASE_LOG_DIR;
    if (is_dir_exist(current_dir) == FAILED)
    {
        agent_utils::write_log(INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log(NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += BASE_LOG_ARCHIVE;
    if (is_dir_exist(current_dir) == FAILED)
    {
        agent_utils::write_log(INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log(NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + std::to_string(cYear);
    if (is_dir_exist(current_dir) == FAILED)
    {
        agent_utils::write_log(INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log(NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + MONTHS[cMonth - 1];
    if (is_dir_exist(current_dir) == FAILED)
    {
        agent_utils::write_log(INVALID_PATH + current_dir, WARNING);
        agent_utils::write_log(NEW_PATH + current_dir, INFO);
        create_dir(current_dir);
    }
    current_dir += "/" + std::to_string(cDay) + "-" + app_name;
    filePath = current_dir;
    if (std::filesystem::exists(current_dir))
    {
        return SUCCESS;
    }
    fstream new_file(filePath, std::ios::out);
    if (new_file)
    {
        new_file.close();
        current_day = cDay;
        current_month = cMonth;
        current_year = cYear;
        return SUCCESS;
    }
    else
    {
        agent_utils::write_log(FCREATION_FAILED + filePath, FAILED);
    }
    return FAILED;
}

int os::delete_file(const string& fileName)
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
            agent_utils::write_log(error, FAILED);
        }
    }
    return FAILED;
}

int os::create_dir(const string dirName)
{
    if (std::filesystem::exists(dirName))
        return SUCCESS;
    else
    {
        try
        {
            if (std::filesystem::create_directory(dirName))
                return SUCCESS;
        }
        catch (const std::exception &e)
        {
            string error(e.what());
            agent_utils::write_log(error, FAILED);
        }
    }
    return FAILED;
}

int os::is_dir_exist(const string& dirName)
{
    if (std::filesystem::exists(dirName))
        return SUCCESS;
    return FAILED;
}

int os::get_regular_files(const string& directory, vector<string> &files)
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
        agent_utils::write_log(except, FAILED);
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

string os::get_json_write_path(const string & type)
{
    string time = agent_utils::get_current_time();
    string filePath = BASE_LOG_DIR;

    if (os::is_dir_exist(filePath) == FAILED)
    {
        os::create_dir(filePath);
    }
    filePath += "json";
    if (os::is_dir_exist(filePath) == FAILED)
    {
        os::create_dir(filePath);
    }
    filePath += "/" + type;
    if (os::is_dir_exist(filePath) == FAILED)
    {
        os::create_dir(filePath);
    }
    filePath += "/" + time + ".json";
    std::ofstream file(filePath);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + filePath, FAILED);
        return "";
    }
    file.close();
    return filePath;
}