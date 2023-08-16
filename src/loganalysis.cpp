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

bool LogAnalysis::isRuleFound(const int ruleId)
{
    auto result = std::find(_idRules.start(), ruleId);
    return (result == _idRules.end()) ? true : false;
}

void LogAnalysis::addMatchedRule(const int ruleId)
{
    this->_idRules(ruleId);
    return;
}

int LogAnalysis::match(LOG_EVENT &logInfo,map<string, map<int, AConfig>> rules)
{
    int result = FAILED;
    bool isRuleMatched = false;
    for (const auto &rule: rules)
    {
        for (const auto &r: rule.second)
        {
            AConfig ruleInfo = r.second;
            P_RULE pRule;
            pRule.id = 0;

            for (int i: this->_processedRules) /* Removing all the processed rules. */
            {
                this->_processingRules.erase(i);
            }

            if (this->_processingRules.size() > 0) /* Checking if rules are about to expire or not. */
            {
                for (int i = 0; i < this->_processingRules.size(); i++)
                {
                    P_RULE pRule = this->_processingRules[i];
                    if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency > 0)
                        || (pRule.end >= AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency <= 0))
                    {
                        this->_processedRules.push_back(i); /* The rule expired */

                    } else if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency <= 0) 
                                || (pRule.end >= AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency <= 0))
                    {
                        /*The rule ID'd and have to alerted*/
                        if (!isRuleFound(pRule.id)) { addMatchedRule(pRule.id); }
                        this->_processedRules.push_back(i);
                    }
                }
            }

            if (!ruleInfo.regex.empty()) /* Checking the regex patterns if exists in the rule */
            {
                if ((result = regexMatch(logInfo.log, ruleInfo.regex)) == SUCCESS)
                {
                    logInfo.log += "|" + std::to_string(ruleInfo.level);
                    cout << logInfo.log << endl;
                    if (!isRuleFound(ruleInfo.id)) { addMatchedRule(ruleInfo.id); }
                }
            }

            if (ruleInfo.max_log_size > 0) /* Validating the syslog size */
            {
                if ((result == isValidSysLog(logInfo.size)) == FAILED)
                {
                    if (!isRuleFound(ruleInfo.id)) { addMatchedRule(ruleInfo.id); }
                }
            }
            
            /*
            <rule id="5702" level="5">
                <if_sid>5700</if_sid>
                <pcre2>^reverse mapping.*failed - POSSIBLE BREAK</pcre2>
                <description>Reverse lookup error (bad ISP or attack).</description>
            </rule>

            <rule id="5703" level="10" frequency="4" timeframe="360">
                <if_matched_sid>5702</if_matched_sid>
                <description>Possible breakin attempt </description>
                <description>(high number of reverse lookup errors).</description>
            </rule>
            */

            if (ruleInfo.frequency > 0 && ruleInfo.timeframe > 0) /*Started the particular log files.*/
            {
                pRule.start = AgentUtils::convertStrToTime(logInfo.timestamp);
                pRule.end   =  pRule.start + ruleInfo.timeframe;
                pRule.frequency = ruleInfo.frequency;
                if (ruleInfo.child_id > 0)
                {
                    pRule.child_id = ruleInfo.child_id;
                    pRule.src_ip   = logInfo.src_ip;
                    if (isRuleFound(pRule.child_id))
                    {
                        pRule.frequency--;
                    }
                }
                if (!ruleInfo.src_ip.empty())
                {
                    pRule.src_ip = ruleInfo.src_ip; /*Some gray part out here.*/
                    if (strcmp(logInfo.c_str(), ))
                }
                // Get the source ip from that log. and store it temproarily.
                // Update the time span to this class members.
                // chek for the time and i            
            }
            
            /*Identify the network related logs and extract its ip, port, ...*/
            /*But the above step should occured in the parser log*/

            if (pRule.id > 0)
            {
                this->_processingRules.push_back(pRule);
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