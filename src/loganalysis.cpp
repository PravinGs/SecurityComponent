#include "service/loganalysis.hpp"

LogAnalysis::LogAnalysis(const string configFile) : _rulesFile(configFile) {}

LogAnalysis::LogAnalysis(){}

void LogAnalysis::setConfigFile(const string configFile)
{
    _rulesFile = configFile;
}

int LogAnalysis::isValidSysLog(size_t size)
{
    return (size <= OS_SIZE_1024) ? SUCCESS : FAILED;
}

LOG_EVENT LogAnalysis::parseToLogInfo(string log, const string format)
{
    /* Log format should be checked before parsing */
    LOG_EVENT logInfo;
    string timestamp, user, program, message;
    std::stringstream ss;
    if (std::count(log.begin(), log.end(), '|') >= 3)
    {
        ss << log;
    }
    else
    {
        string fLog = formatSysLog(log, format);
        if (fLog.empty()) { return logInfo; }
        ss << fLog;
    }
    std::getline(ss, timestamp, '|');
    std::getline(ss, user, '|');
    std::getline(ss, program, '|');
    std::getline(ss, message, '|');
    /* Set the values to the structure */
    logInfo.format = format;
    logInfo.size = log.size();
    logInfo.timestamp = timestamp;
    logInfo.user = user;
    logInfo.program = program;
    logInfo.log = message;

    return logInfo;
}

int LogAnalysis::regexMatch(const string log, const string pattern)
{
    std::regex r(pattern);
    std::smatch matches;
    return (regex_search(log, matches, r)) ? SUCCESS : FAILED;
}

int LogAnalysis::match(LOG_EVENT &logInfo,map<string, map<int, AConfig>> rules)
{
    int result = FAILED;
    for (const auto &rule: rules)
    {
        for (const auto &r: rule.second)
        {
            AConfig ruleInfo = r.second;
            if (!ruleInfo.regex.empty())
            {
                if ((result = regexMatch(logInfo.log, ruleInfo.regex)) == SUCCESS)
                {
                    logInfo.log += "|" + std::to_string(ruleInfo.level);
                    cout << logInfo.log << endl;
                }
            }
        }
    }
    return result;
}

int LogAnalysis::getRegularFiles(const string directory, vector<string> &files)
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
                files.push_back(parent + child);
            }
            else if (std::filesystem::is_directory(entry.path()))
            {
                string child = entry.path();
                result = getRegularFiles(parent + child, files);
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

int LogAnalysis::analyseFile(const string file, string format)
{
    int result;
    string line;
    map<string, map<int, AConfig>> rules;
    fstream fp(file, std::ios::in);
    if (!fp)
    {
        AgentUtils::writeLog(FILE_ERROR + file, FAILED);
        return FAILED;
    }

    if ((result = _configService.readRuleConfig(_rulesFile, rules)) == FAILED)
    {
        return result;
    }
    AgentUtils::writeLog("Log analysis started for " + file, INFO);
    while(std::getline(fp, line))
    {
        if (line.empty()) continue;
        LOG_EVENT logInfo = parseToLogInfo(line, format);
        result = match(logInfo, rules);
    }
    
    fp.close();
    return result;
}

int LogAnalysis::start(const string path)
{
    string format;
    if (path.find("dpkg") != string::npos)
    {
        format = "dpkg";
    }
    else if (path.find("auth") != string::npos)
    {
        format = "syslog";
    }
    int result = SUCCESS;
    int isFile = 0;
    vector<string> files;
    if (OS::isDirExist(path) && std::filesystem::is_regular_file(path))
    {
        isFile = 1;
    }

    if (isFile)
    {
        result = analyseFile(path, format);
    }
    else
    {
        string currentFile = path;
        if (currentFile[currentFile.size() - 1] != '/')
        {
            currentFile += "/";
        }
        result = getRegularFiles(currentFile, files);

        if (result == FAILED) return FAILED;
        
        if (files.size() == 0)
        {
            AgentUtils::writeLog("Check directory ( " + path + " ). It contains no files.", FAILED);
            return FAILED;
        }
        for (string file : files)
        {
            result = analyseFile(file, format);
        }
    }
    return result;
}

string LogAnalysis::formatSysLog(string log, const string format)
{
    string fLog, token, formattedTime;
    int index = 0;
    string currentTime = log.substr(0, 15);     
    std::stringstream stream; 
    if (format == "dpkg")            
    {
        string temp;
        fLog += log.substr(0,19);
        temp = log.substr(20);
        fLog += "|" + temp.substr(0, temp.find(' '));
        temp = temp.substr(temp.find(' ') + 1);
        fLog += "|" + temp;
        cout << fLog << endl;
        return fLog;
    }
    else
    {
        AgentUtils::convertTimeFormat(currentTime, formattedTime);
        stream << log.substr(16);
    }
    fLog += formattedTime;
    while (std::getline(stream, token, ' ') && index < 3)
    {
        if (index == 2)
        {
            string message = token;
            while (std::getline(stream, token, ' '))
            {
                message += ' ' + token;
            }
            fLog += '|' + message;
            index = 4;
            continue;
        }
        fLog += '|' + token;
        index++;
    }
    cout << fLog  << endl;

    return fLog;
}