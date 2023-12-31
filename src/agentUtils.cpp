#include "agentUtils.hpp"

int OS::CurrentDay = 0;
int OS::CurrentMonth = 0;
int OS::CurrentYear = 0;
bool AgentUtils::syslog_enabled = true;
fstream AgentUtils::logfp;
std::mutex logMutex;
bool AgentUtils::debug = false;


void AgentUtils::setupLogger()
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
            AgentUtils::syslog_enabled = false;
        }
    } 
    else
    {
        AgentUtils::syslog_enabled = false;
    }
}

string AgentUtils::trim(string line)
{
    const auto strBegin = line.find_first_not_of(" \t");
    if (strBegin == string::npos)
        return "";

    const auto strEnd = line.find_last_not_of(" \t");
    const auto strRange = strEnd - strBegin + 1;

    string str = line.substr(strBegin, strRange);
    return (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') ? str.substr(1, str.length() - 2) : str;
}

void AgentUtils::updateLogWrittenTime(const string& appName, const string& time)
{
    string filePath = BASE_CONFIG_DIR;
    if (OS::isDirExist(filePath) == FAILED)
    {
        OS::createDir(filePath);
    }
    filePath += BASE_CONFIG_TMP;
    if (OS::isDirExist(filePath) == FAILED)
    {
        OS::createDir(filePath);
    }
    filePath += appName;
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        writeLog(INVALID_FILE + filePath, FAILED);
        return;
    }
    file << time;
    writeLog("Time " + time + " updated to " + filePath, SUCCESS);
    file.close();
    return;
}

int AgentUtils::getHostName(string &host)
{
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        hostname[strlen(hostname)] = '\0';
        host = hostname;
    }
    else
    {
        writeLog("Failed to get the hostname.", FAILED);
        return FAILED;
    }
    return SUCCESS;
}

string AgentUtils::getCurrentTime()
{
    char currentTime[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", t);
    return currentTime;
}

bool AgentUtils::isValidTimeString(const std::string& timeString) {
    std::tm tm_struct = {};
    return (strptime(timeString.c_str(), "%b %e %H:%M:%S", &tm_struct) != nullptr) ? true : false;
}

int AgentUtils::convertTimeFormat(const std::string &inputTime, std::string &formatTime)
{
    if (!isValidTimeString(inputTime)) { return FAILED; }
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

void AgentUtils::writeLog(const string& log)
{
    string time = getCurrentTime();
    string line = time + " " + log + "\n";
    std::lock_guard<std::mutex> lm(logMutex);
    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.write(line.c_str(), line.size());
    }
    AgentUtils::backupLogFile();
    return;
}

void AgentUtils::writeLog(const string& log, int logLevel)
{
    string time = getCurrentTime();
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
            if (AgentUtils::logfp.is_open())
            {
                AgentUtils::logfp.write(line.c_str(), line.size());
                AgentUtils::logfp.flush();
            }
        }
    }
    AgentUtils::backupLogFile();
    return;
}

void AgentUtils::backupLogFile()
{
    std::streampos size;
    if (AgentUtils::logfp.is_open()){
        AgentUtils::logfp.seekg(0, std::ios::end);
        size = AgentUtils::logfp.tellg();
    }
    if (size > 1024)
    {
        OS::compressFile(LOG_PATH);
        AgentUtils::logfp.close();
        std::filesystem::remove(LOG_PATH);
        AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    }
}

int OS::compressFile(const string& logFile)
{
    string currentFile = logFile;
    if (logFile == LOG_PATH){
        vector<string> files;
        OS::getRegularFiles(BASE_LOG_DIR, files);
        currentFile += std::to_string(files.size());
    }
    int result = SUCCESS;
    if (currentFile.size() == 0)
    {
        AgentUtils::writeLog("No global log file updated in the code for OS", FAILED);
        return FAILED;
    }
    fstream file(logFile, std::ios::in | std::ios::binary);
    if (!file)
    {
        AgentUtils::writeLog("No file exist for backup ( " + logFile + " )", FAILED);
        return FAILED;
    }
    gzFile zLog;
    string zipFile = currentFile + ".gz";
    zLog = gzopen(zipFile.c_str(), "w");
    if (!zLog)
    {
        AgentUtils::writeLog(FCREATION_FAILED + zipFile, FAILED);
        file.close();
        return FAILED;
    }
    string line;
    while (std::getline(file, line))
    {
        if (line.size() == 0)
            continue;
        if (gzwrite(zLog, line.c_str(), static_cast<unsigned int>(line.size())) != (int)line.size())
        {
            AgentUtils::writeLog(FWRITE_FAILED + zipFile, FAILED);
            result = FAILED;
            break;
        }
    }
    file.close();
    gzclose(zLog);
    if (result == SUCCESS)
    {
        OS::deleteFile(currentFile);
    }
    else
    {
        AgentUtils::writeLog(FDELETE_FAILED + currentFile, FAILED);
    }
    return result;
}

string OS::isEmpty(string filename)
{
    string nonEmptyPath;
    if (std::filesystem::exists(filename))
    {
        std::ifstream file(filename, std::ios::binary);
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();

        if (fileSize == 0)
        {
            nonEmptyPath = filename + ".1";
        }
        else
        {
            nonEmptyPath = filename;
        }
        file.close();
    }
    else
    {
       nonEmptyPath = "";
    }
    return nonEmptyPath;
}

string OS::getCurretDayFileByName(const string& appName)
{
    string currentLogFile = BASE_LOG_DIR;
    currentLogFile       += BASE_LOG_ARCHIVE;
    currentLogFile += std::to_string(CurrentYear) + "/" + MONTHS[CurrentMonth-1] + "/" + std::to_string(CurrentDay) + "-" + appName;
    return currentLogFile;
}

int OS::handleLocalLogFile(int day, int month, int year, string& filePath, const string& appName)
{
    int result;
    string currentDir = BASE_LOG_DIR;
    if (day != CurrentDay)
    {
        string currentLogFile = getCurretDayFileByName(appName);
        
        if ((result = compressFile(currentLogFile)) == FAILED)
        {
            return FAILED;
        }
        result = createLogFile(day, month, year, filePath, appName);
    }
    else
    {
        result = createLogFile(day, month, year, filePath, appName);
    }

    return result;
}

int OS::createLogFile(int cDay, int cMonth, int cYear, string& filePath, const string& appName)
{
    string currentDir = BASE_LOG_DIR;
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(INVALID_PATH + currentDir, WARNING);
        AgentUtils::writeLog(NPATH + currentDir, INFO);
        createDir(currentDir);
    }
    currentDir += BASE_LOG_ARCHIVE;
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(INVALID_PATH + currentDir, WARNING);
        AgentUtils::writeLog(NPATH + currentDir, INFO);
        createDir(currentDir);
    }
    currentDir += "/" + std::to_string(cYear);
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(INVALID_PATH + currentDir, WARNING);
        AgentUtils::writeLog(NPATH + currentDir, INFO);
        createDir(currentDir);
    }
    currentDir += "/" + MONTHS[cMonth - 1];
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(INVALID_PATH + currentDir, WARNING);
        AgentUtils::writeLog(NPATH + currentDir, INFO);
        createDir(currentDir);
    }
    currentDir += "/" + std::to_string(cDay) + "-" + appName;
    filePath = currentDir;
    if (std::filesystem::exists(currentDir))
    {
        return SUCCESS;
    }
    fstream nFile(filePath, std::ios::out);
    if (nFile)
    {
        nFile.close();
        CurrentDay = cDay;
        CurrentMonth = cMonth;
        CurrentYear = cYear;
        return SUCCESS;
    }
    else
    {
        AgentUtils::writeLog(FCREATION_FAILED + filePath, FAILED);
    }
    return FAILED;
}

int OS::deleteFile(const string& fileName)
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
            AgentUtils::writeLog(error, FAILED);
        }
    }
    return FAILED;
}

int OS::createDir(const string dirName)
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
            AgentUtils::writeLog(error, FAILED);
        }
    }
    return FAILED;
}

int OS::isDirExist(const string& dirName)
{
    if (std::filesystem::exists(dirName))
        return SUCCESS;
    return FAILED;
}

int OS::readRegularFiles(vector<string> &files)
{
    string directory = BASE_LOG_DIR;
    directory += "josn/";
    if (!std::filesystem::exists(directory))
    {
        AgentUtils::writeLog(INVALID_PATH + directory, FAILED);
        return FAILED;
    }
    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        string filePath = directory;
        if (std::filesystem::is_regular_file(entry))
        {
            filePath += entry.path().string();
            files.push_back(filePath);
        }
    }

    if (files.size() == 0)
    {
        AgentUtils::writeLog(INVALID_PATH + directory, FAILED);
        return FAILED;
    }

    return SUCCESS;
}

int OS::getRegularFiles(const string& directory, vector<string> &files)
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
        AgentUtils::writeLog(except, FAILED);
    }
    return result;
}

std::time_t AgentUtils::convertStrToTime(const string &datetime)
{
    const char *STANDARD_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, STANDARD_TIME_FORMAT);
    return std::mktime(&tm);
}

string OS::getJsonWritePath(const string & type)
{
    string time = AgentUtils::getCurrentTime();
    string filePath = BASE_LOG_DIR;

    if (OS::isDirExist(filePath) == FAILED)
    {
        OS::createDir(filePath);
    }
    filePath += "json";
    if (OS::isDirExist(filePath) == FAILED)
    {
        OS::createDir(filePath);
    }
    filePath += "/" + type;
    if (OS::isDirExist(filePath) == FAILED)
    {
        OS::createDir(filePath);
    }
    filePath += "/" + time + ".json";
    std::ofstream file(filePath);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + filePath, FAILED);
        return "";
    }
    file.close();
    return filePath;
}