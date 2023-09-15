#include "service/logservice.hpp"

string toLowerCase(string &str)
{
    string lowerCaseString;
    for (char c : str)
    {
        lowerCaseString += std::tolower(c);
    }
    return lowerCaseString;
}

int LogService::_saveAsJSON(Json::Value &json, const string &path, const vector<string> &logs, const vector<string> &columns, const char &delimeter)
{
    string jsonPath = path;
    if (verifyJsonPath(jsonPath) == FAILED)
    {
        return FAILED;
    }
    fstream file(jsonPath, std::ios::out);
    Json::StreamWriterBuilder writerBuilder;
    if (!file)
    {
        AgentUtils::writeLog(FWRITE_FAILED + jsonPath, FAILED);
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
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(json, &file);
    file.close();
    AgentUtils::writeLog(FWRITE_SUCCESS + jsonPath, SUCCESS);
    return SUCCESS;
}

void LogService::categorize(string &line, const vector<string> &levels)
{
    int maxLevel = 0;
    string network = "network";
    string ufw = "ufw";
    string lowerLine = line;
    for (string level : levels)
    {
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        if (lowerLine.find(level) != std::string::npos)
        {
            if (_logLevel[toLowerCase(level)] > maxLevel)
            {
                maxLevel = _logLevel[toLowerCase(level)];
            }
        }
    }

    line += "|" + std::to_string(maxLevel);

    if (lowerLine.find(network) != std::string::npos)
    {
        line += "|" + network;
    }
    else if (lowerLine.find(ufw) != std::string::npos)
    {
        line += "|" + ufw;
    }
    else
    {
        line += "|sys";
    }

    return;
}

bool filterLog(string &line, vector<string> levels)
{
    bool result = false;
    string lowerLine = line;
    for (string level : levels)
    {
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        if (lowerLine.find(level) != std::string::npos)
        {
            result = true;
        }
    }
    return result;
}

int LogService::_readSysLog(const string &path, vector<string> &logs, const char &delimeter, const string &previousTime, bool &flag, const vector<string> &levels, string &nextReadingTime)
{
    int result = SUCCESS;
    const string sep = "|";
    string formattedTime, line;
    std::time_t lastWrittenTime = AgentUtils::convertStrToTime(previousTime);

    fstream file(path, std::ios::in);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        string currentTime = line.substr(0, 15);                         /* Extract the date time format fromt the line */
        AgentUtils::convertTimeFormat(currentTime, formattedTime);       /* This func convert to standard time format */
        std::time_t cTime = AgentUtils::convertStrToTime(formattedTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }
        string log, token;
        log += formattedTime;
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
                categorize(log, levels);
                index = 4;
                continue;
            }
            log += sep + token;
            index++;
        }

        logs.push_back(log);
        std::time_t tempTime = AgentUtils::convertStrToTime(nextReadingTime);
        if (cTime > tempTime)
        {
            nextReadingTime = formattedTime;
        }
        if (cTime == lastWrittenTime)
        {
            flag = false;
        }
    }

    file.close();

    if (flag)
    {
        _readSysLog(path + ".1", logs, delimeter, previousTime, flag, levels, nextReadingTime);
    }
    return result;
}

int LogService::getSysLog(const string &appName, Json::Value &json, const vector<string> &names, const string &path, string &previousTime, const vector<string> &levels, const char &remote)
{
    string logDir = BASE_LOG_DIR;
    logDir += BASE_LOG_ARCHIVE;
    const char delimeter = ' ';
    vector<string> logs;
    string nextReadingTime = previousTime;
    bool flag = true;
    int result;

    if (strcmp(appName.c_str(), "syslog") == 0 || strcmp(appName.c_str(), "auth") == 0)
    {

        if (remote == 'y' || remote == 'Y')
        {
            UdpQueue queue;
            result = readRemoteSysLog(queue, logs);
            queue.stop();
        }
        else
        {
            result = _readSysLog(path, logs, delimeter, previousTime, flag, levels, nextReadingTime);
        }
        if (logs.size() == 0 || result == FAILED)
        {
            AgentUtils::writeLog("Read 0 logs for" + appName);
        }
        else
        {
            previousTime = nextReadingTime;
            AgentUtils::writeLog(appName + " logs collected", INFO);
            if (_configService.toVector(logs[0], '|').size() < names.size())
            {
                AgentUtils::writeLog("Invalid Log Attributes configured for " + appName, FAILED);
                result = FAILED;
            }
            else
            {
                string fileName = nextReadingTime + "-" + appName;
                result = _saveAsJSON(json, fileName, logs, names, '|');
            }

            AgentUtils::writeLog("Storing " + appName + " logs started", INFO);
            if ((result = saveToLocal(logs, appName)) == SUCCESS)
            {

                AgentUtils::writeLog(FWRITE_SUCCESS + logDir, INFO);
            }
            else
            {
                AgentUtils::writeLog(FWRITE_FAILED + logDir, FAILED);
            }
        }
    }
    else if (strcmp(appName.c_str(), "dpkg") == 0)
    {
        result = readDpkgLog(path, logs, previousTime, nextReadingTime, flag);
        previousTime = nextReadingTime;
        if (logs.size() == 0)
        {
            AgentUtils::writeLog("Read 0 logs for " + appName);
        }
        else
        {
            AgentUtils::writeLog("Storing " + appName + " logs started", INFO);
            if ((result = saveToLocal(logs, appName)) == SUCCESS)
            {
                AgentUtils::writeLog(FWRITE_SUCCESS + logDir, INFO);
            }
            else
            {
                AgentUtils::writeLog(FWRITE_FAILED + logDir, FAILED);
            }
        }
    }

    return result;
}

int LogService::getAppLog(Json::Value &json, const vector<string> &names, const string &readDir, const string &writePath, string &previousTime, const vector<string> &levels, const char &delimeter)
{
    vector<string> logs;
    bool flag = true;
    vector<std::filesystem::directory_entry> files = _getDirFiles(readDir);
    int index = (int)files.size() - 1;
    string nextReadingTime = previousTime;
    while (flag && index >= 0)
    {
        string path = files[index].path();
        if (!std::filesystem::is_regular_file(path))
        {
            index--;
            continue;
        }

        if (_readAppLog(path, logs, delimeter, previousTime, flag, levels, nextReadingTime) == FAILED || logs.size() == 0)
        {
            return FAILED;
        }
        AgentUtils::writeLog("Applog collected from " + path, INFO);
        index--;
    }

    if (_configService.toVector(logs[0], delimeter).size() < names.size())
    {
        AgentUtils::writeLog("Invalid Log Attributes configured for Applog", FAILED);
        return FAILED;
    }
    previousTime = nextReadingTime;
    return _saveAsJSON(json, writePath, logs, names, delimeter);
}

int LogService::_readAppLog(const string &path, vector<string> &logs, const char &delimeter, const string &previousTime, bool &flag, const vector<string> &levels, string &nextReadingTime)
{
    fstream file(path);
    string line;
    std::time_t lastWrittenTime = AgentUtils::convertStrToTime(previousTime);
    bool isCriticalLog = false;
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        if (line.length() == 0)
        {
            continue;
        }

        string currentTime = line.substr(0, 19);                       /* Extract the date time format fromt the line */
        std::time_t cTime = AgentUtils::convertStrToTime(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }

        isCriticalLog = filterLog(line, levels);
        if (isCriticalLog)
        {
            logs.push_back(line);
        }
        std::time_t tempTime = AgentUtils::convertStrToTime(nextReadingTime);
        if (cTime > tempTime)
        {
            nextReadingTime = currentTime;
        }
        if (cTime == lastWrittenTime)
        {
            flag = false;
        }
    }
    file.close();
    return SUCCESS;
}

vector<std::filesystem::directory_entry> LogService::_getDirFiles(const string &directory)
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

int LogService::saveToLocal(const vector<string> &logs, const string &appName)
{
    string filePath;
    auto today = std::chrono::system_clock::now();
    auto timeInfo = std::chrono::system_clock::to_time_t(today);
    std::tm *tm_info = std::localtime(&timeInfo);

    int day = tm_info->tm_mday;
    int month = tm_info->tm_mon + 1;
    int year = tm_info->tm_year + 1900;

    if (OS::handleLocalLogFile(day, month, year, filePath, appName) == FAILED)
    {
        return FAILED;
    }

    fstream file(filePath, std::ios::app);

    for (string line : logs)
    {
        file << line << "\n";
    }

    file.close();

    return SUCCESS;
}

int LogService::verifyJsonPath(string &timestamp)
{
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

    filePath += "/" + timestamp + ".json";

    std::ofstream file(filePath);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + filePath, FAILED);
        return FAILED;
    }
    file.close();
    timestamp = filePath;
    return SUCCESS;
}

int LogService::readDpkgLog(const string &path, vector<string> &logs, string &previousTime, string &nextReadingTime, bool &flag)
{
    string formattedTime, line;
    std::time_t lastWrittenTime = AgentUtils::convertStrToTime(previousTime);

    fstream file(path, std::ios::in);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        string log, temp, host;
        AgentUtils::getHostName(host);
        string currentTime = line.substr(0, 19);
        std::time_t cTime = AgentUtils::convertStrToTime(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }

        log += currentTime;
        log += "|"+host;
        fLog += "|" + "dpkg";
        temp = line.substr(20);
        log += "|"+temp;
        logs.push_back(log);

        std::time_t tempTime = AgentUtils::convertStrToTime(nextReadingTime);
        if (cTime > tempTime)
        {
            nextReadingTime = currentTime;
        }
        if (cTime == lastWrittenTime)
        {
            flag = false;
        }
    }
    if (flag)
    {
        readDpkgLog(path + ".1", logs, previousTime, nextReadingTime, flag);
    }
    return SUCCESS;
}

int LogService::readRemoteSysLog(UdpQueue &queue, vector<string> &logs)
{
    return queue.getMessage(logs);
}

LogService::~LogService() {}