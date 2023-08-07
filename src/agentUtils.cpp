#include "agentUtils.hpp"

int OS::CurrentDay = 0;
int OS::CurrentMonth = 0;
int OS::CurrentYear = 0;

string AgentUtils::trim(string line)
{
    const auto strBegin = line.find_first_not_of(" \t");
    if (strBegin == std::string::npos)
        return "";

    const auto strEnd = line.find_last_not_of(" \t");
    const auto strRange = strEnd - strBegin + 1;

    string str = line.substr(strBegin, strRange);
    return (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') ? str.substr(1, str.length() - 2) : str;
}

void AgentUtils::updateLogWrittenTime(const string appName, const string time)
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
        writeLog("Failed to open " + filePath + " check filepath and permission", FAILED);
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

int AgentUtils::convertTimeFormat(const std::string &inputTime, std::string &formatTime)
{
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
    return 0;
}

void AgentUtils::writeLog(string log)
{
    const string filePath = "/etc/scl/log/agent.log";
    string time = getCurrentTime();
    std::fstream file(filePath, std::ios::app);
    file << time << "  " << log << endl;
    // syslog(LOG_USER, "%s, ", log.c_str());
    return;
}

void AgentUtils::writeLog(string log, int logLevel)
{
    const string filePath = "/etc/scl/log/agent.log";
    string time = getCurrentTime();
    std::fstream file(filePath, std::ios::app);
    switch (logLevel)
    {
    case SUCCESS:
        file << time << " : SUCCESS : " << log << endl;
        // syslog(LOG_INFO, "SUCCESS : %s",log.c_str());
        break;
    case FAILED:
        file << time << " : FAILED : " << log << endl;
        // syslog(LOG_INFO, "FAILED : %s", log.c_str());
        break;
    case WARNING:
        file << time << " : WARNING : " << log << endl;
        // syslog(LOG_USER, "WARNING: %s", log.c_str());
        break;
    case CRITICAL:
        file << time << " : CRITICAL : " << log << endl;
        break;
    default:
        break;
    }
    file.close();
    return;
}

int OS::compressFile(const string logFile)
{
    string currentFile = logFile;
    int result = SUCCESS;
    if (currentFile.size() == 0)
    {
        AgentUtils::writeLog("No global log file updated in the code for OS", FAILED);
        cerr << "Empty Global file, 265" << endl;
        return FAILED;
    }
    fstream file(currentFile, std::ios::in | std::ios::binary);
    if (!file)
    {
        AgentUtils::writeLog("No file exist for backup ( " + currentFile + " )", FAILED);
        cerr << "No file to compress, 271" << endl;
        return FAILED;
    }
    gzFile zLog;
    string zipFile = currentFile + ".gz";
    zLog = gzopen(zipFile.c_str(), "w");
    if (!zLog)
    {
        AgentUtils::writeLog("Failed to create zip file ( " + zipFile + " )", FAILED);
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
            AgentUtils::writeLog("Failed to write compressed file ( " + zipFile + " )", FAILED);
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
        AgentUtils::writeLog("Faile to delete old archived log file ( " + currentFile + " )", FAILED);
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
            std::cout << "The file is empty." << std::endl;
            nonEmptyPath = filename + ".1";
        }
        else
        {
            std::cout << "The file is not empty. Size: " << fileSize << " bytes." << std::endl;
            nonEmptyPath = filename;
        }
        file.close();
    }
    else
    {
        std::cout << "The file does not exist." << std::endl;
    }
    return nonEmptyPath;
}

string OS::getCurretDayFileByName(string appName)
{
    string currentLogFile = BASE_LOG_DIR;
    currentLogFile       += BASE_LOG_ARCHIVE;
    currentLogFile += std::to_string(CurrentYear) + "/" + MONTHS[CurrentMonth-1] + "/" + std::to_string(CurrentDay) + "-" + appName;
    return currentLogFile;
}

int OS::handleLocalLogFile(int day, int month, int year, string &filePath, const string appName)
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

int OS::createLogFile(int cDay, int cMonth, int cYear, string &filePath, const string appName)
{
    string currentDir = BASE_LOG_DIR;
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(currentDir + " Not exist, creating new directory", WARNING);
        createDir(currentDir);
    }
    currentDir += BASE_LOG_ARCHIVE;
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(currentDir + " Not exist, creating new directory", WARNING);
        createDir(currentDir);
    }
    currentDir += "/" + std::to_string(cYear);
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(currentDir + " Not exist, creating new directory", WARNING);
        createDir(currentDir);
    }
    currentDir += "/" + MONTHS[cMonth - 1];
    if (isDirExist(currentDir) == FAILED)
    {
        AgentUtils::writeLog(currentDir + " Not exist, creating new directory", WARNING);
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
        AgentUtils::writeLog("Failed to create ( " + filePath + " ) check permission", FAILED);
        cerr << "Failed to create file check permission" << endl;
    }
    return FAILED;
}

int OS::deleteFile(const string fileName)
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
            // std::cerr << e.what() << '\n';
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
            std::cerr << e.what() << '\n';
        }
    }
    return FAILED;
}

int OS::isDirExist(const string dirName)
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
        // cerr << "Invalid directory to read regular files" << endl;
        AgentUtils::writeLog("Invalid directory to read regular files ( " + directory + " )", FAILED);
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
        // cerr << "No regular file exists in " << directory << endl;
        AgentUtils::writeLog("No regular file exists in (" + directory + " )", WARNING);
        return FAILED;
    }

    return SUCCESS;
}