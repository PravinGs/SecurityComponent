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

std::time_t LogService::_convertToTime(const string &datetime)
{
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, STANDARD_TIME_FORMAT);
    return std::mktime(&tm);
}

int LogService::_saveAsJSON(Json::Value &json, string path, const vector<string> logs, const vector<string> columns, char delimeter)
{
    if (verifyJsonPath(path) == FAILED)
    {
        return FAILED;
    }
    fstream file(path, std::ios::out);
    Json::StreamWriterBuilder writerBuilder;
    if (!file)
    {
        AgentUtils::writeLog(FWRITE_FAILED + path, FAILED);
        return FAILED;
    }

    json["LogObjects"] = Json::Value(Json::arrayValue);
    for (auto log : logs)
    {
        vector<string> splitedLogs = _configService.toVector(log, delimeter);
        Json::Value jsonLog;
        for (int i = 0; i < (int)columns.size(); i++)
        {
            jsonLog["LogCategory"] = 0;
            if (columns[i] == "LogCategory")
            {
                jsonLog[columns[i]] = _logCategory[splitedLogs[i]];
                continue;
            }
            if (columns[i] == "LogLevel")
            {
                int level = _logLevel[toLowerCase(splitedLogs[i])];
                int priorVal;
                jsonLog[columns[i]] = level;
                if (level <= 2)
                {
                    priorVal = 1;
                }
                else if (level == 3)
                {
                    priorVal = 2;
                }
                else if (level > 3)
                {
                    priorVal = 3;
                }
                jsonLog["Priority"] = priorVal;
                continue;
            }
            if (columns[i] == "Priority")
            {
                jsonLog[columns[i]] = _priorityLevel[toLowerCase(splitedLogs[i])];
                continue;
            }
            jsonLog[columns[i]] = splitedLogs[i];
        }
        json["LogObjects"].append(jsonLog);
    }
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(json, &file);
    file.close();
    AgentUtils::writeLog("Log written to " + path, SUCCESS);
    return SUCCESS;
}

bool LogService::isPriorityLog(string &line, vector<string> levels, Json::Value &json)
{
    bool result = false;
    string levelOut;
    int maxLevel = -1;
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
            result = true;
            if (_logLevel[toLowerCase(level)] > maxLevel)
            {
                maxLevel = _logLevel[toLowerCase(level)];
                levelOut = level;
            }
        }
    }
    if (maxLevel == -1)
    {
        line += "|none";
    }
    else
    {
        line += "|" + levelOut;
    }
    if (lowerLine.find(network) != std::string::npos)
    {
        line += "|network";
    }
    else if (lowerLine.find(ufw) != std::string::npos)
    {
        line += "|ufw";
    }
    else
    {
        line += "|sys";
    }

    return result;
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

int LogService::_readSysLog(Json::Value &json, string path, vector<string> &logs, const char delimeter, string &previousTime, bool &flag, vector<string> levels, string &nextReadingTime)
{
    const string sep = "|";
    string formattedTime, line;
    std::time_t lastWrittenTime = _convertToTime(previousTime);

    fstream file(path, std::ios::in);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        string currentTime = line.substr(0, 15);                   /* Extract the date time format fromt the line */
        AgentUtils::convertTimeFormat(currentTime, formattedTime); /* This func convert to standard time format */
        std::time_t cTime = _convertToTime(formattedTime);         /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }

        string log, token;
        std::stringstream stream(line);
        int index = 0;
        bool isCriticalLog = false;
        while (std::getline(stream, token, delimeter) && index < 4)
        {
            /* To skip the line after date and time */
            if (index == 0)
            {
                while (index < 2 && std::getline(stream, token, delimeter))
                {
                    if (token.size() != 0)
                    {
                        index++;
                    }
                }
                log += formattedTime;
                index = 1;
                continue;
            }
            /*Accumulate all the spaces seperated words into one message*/
            if (index == 3)
            {
                string message = token;
                while (std::getline(stream, token, delimeter))
                {
                    message += ' ' + token;
                }
                isCriticalLog = isPriorityLog(message, levels, json);
                log += sep + message;
                index = 4;
                continue;
            }
            log += sep + token;
            index++;
        }
        if (isCriticalLog)
        { /**/
        }
        logs.push_back(log);
        std::time_t tempTime = _convertToTime(nextReadingTime);
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
        _readSysLog(json, path + ".1", logs, delimeter, previousTime, flag, levels, nextReadingTime);
    }
    return SUCCESS;
}

int LogService::getSysLog(string appName, Json::Value &json, vector<string> names, const string path, string &previousTime, vector<string> levels, const char remote)
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
            result = _readSysLog(json, path, logs, delimeter, previousTime, flag, levels, nextReadingTime);
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
                result = _saveAsJSON(json, nextReadingTime + "-" + appName, logs, names, '|');
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

int LogService::getAppLog(Json::Value &json, vector<string> names, const string readDir, const string writePath, string &previousTime, vector<string> levels, const char delimeter)
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

        if (_readAppLog(json, path, logs, delimeter, previousTime, flag, levels, nextReadingTime) == FAILED || logs.size() == 0)
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

int LogService::_readAppLog(Json::Value &json, string path, vector<string> &logs, const char delimeter, const string previousTime, bool &flag, vector<string> levels, string &nextReadingTime)
{
    fstream file(path);
    string line;
    std::time_t lastWrittenTime = _convertToTime(previousTime);
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

        string currentTime = line.substr(0, 19);         /* Extract the date time format fromt the line */
        std::time_t cTime = _convertToTime(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }

        isCriticalLog = filterLog(line, levels);
        if (isCriticalLog)
        {
            logs.push_back(line);
        }
        std::time_t tempTime = _convertToTime(nextReadingTime);
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

vector<std::filesystem::directory_entry> LogService::_getDirFiles(const string directory)
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

int LogService::saveToLocal(vector<string> logs, string appName)
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
        file << line << endl;
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

int LogService::readDpkgLog(const string path, vector<string> &logs, string &previousTime, string &nextReadingTime, bool &flag)
{
    string formattedTime, line;
    std::time_t lastWrittenTime = _convertToTime(previousTime);

    fstream file(path, std::ios::in);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return FAILED;
    }

    while (std::getline(file, line))
    {
        string log, temp;
        string currentTime = line.substr(0, 19);
        std::time_t cTime = _convertToTime(currentTime); /* Convert string time to time_t format for comparision between time_t objects */
        if (cTime < lastWrittenTime)
        {
            continue;
        }
        log += currentTime;
        temp = line.substr(20);
        log += "|" + temp.substr(0, temp.find(' '));
        temp = temp.substr(temp.find(' ') + 1);
        log += "|" + temp;
        logs.push_back(log);

        std::time_t tempTime = _convertToTime(nextReadingTime);
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