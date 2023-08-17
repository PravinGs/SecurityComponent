#include "service/loganalysis.hpp"

LogAnalysis::LogAnalysis(const string configFile) : _rulesFile(configFile) {}

LogAnalysis::LogAnalysis(){}

void extractNetworkLog(LOG_EVENT &logInfo)
{
    /* IN= OUT=lo SRC=127.0.0.1 DST=127.0.0.1 LEN=52 TOS=0x00 PREC=0x00 
    TTL=64 ID=38860 DF PROTO=TCP SPT=8888 DPT=55764 WINDOW=512 RES=0x00 ACK URGP=0 */

    string temp = logInfo.log;
    string src_ip = temp.substr(temp.find("SRC") + 4);
    src_ip = src_ip.substr(0, src_ip.find_first_of(' '));
    string dest_ip = temp.substr(temp.find("DST") + 4);
    dest_ip = dest_ip.substr(0, dest_ip.find_first_of(' '));
    string proto = temp.substr(temp.find("PROTO") + 6);
    proto = proto.substr(0, proto.find_first_of(' '));

    logInfo.src_ip  = src_ip;
    logInfo.dest_ip = dest_ip;
    logInfo.proto   = proto;

    return;

}

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
    logInfo.rule_id = 0;
    logInfo.is_matched = 0;
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

    if (logInfo.log.find("SRC=") != string::npos)
    {
        extractNetworkLog(logInfo);
    }

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
    auto result = std::find(_idRules.begin(), _idRules.end(), ruleId);
    return (result == _idRules.end()) ? true : false;
}

void LogAnalysis::addMatchedRule(const int ruleId)
{
    if (!isRuleFound(ruleId)) { addMatchedRule(ruleId); }
    return;
}

int LogAnalysis::match(LOG_EVENT &logInfo,map<string, map<int, AConfig>> rules)
{
    /*
        read the rules one by one
        checking if any rules in the memory, which is expired if it is remove it,
        checking any rules is processing state wait for logs
        Checking and do the regex matching for the logs
        Checking the read syslog size. 
        Checking any rule which have child id, that might be identified before.
        Checking the timeframe or frequency is available for the rule if it is, do the given.
        And during those checkings if rule matched add the rule id to the Id'd rules.
    */
    int result = FAILED;
    for (auto &rule: rules)
    {
        for (auto &r: rule.second)
        {
            AConfig ruleInfo = r.second;
            P_RULE pRule;
            pRule.id = 0;
            pRule.if_sid = 0;
            pRule.same_source_ip = 0;

            for (int i : this->_processedRules) /* Removing all the processed rules. */
            {
                this->_processingRules.erase(this->_processingRules.begin() + this->_processedRules[i]);
            }

            if (this->_processingRules.size() > 0) /* Checking if rules are about to expire or not. */
            {
                for (int i = 0; i < (int)this->_processingRules.size(); i++)
                {
                    P_RULE pRule = this->_processingRules[i];
                    if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.d_frequency != pRule.frequency))
                    {
                        this->_processedRules.push_back(i); /* The rule expired */

                    } else if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency == pRule.d_frequency) 
                                || pRule.frequency <= pRule.d_frequency)
                    {
                        /*The rule ID'd and have to alerted*/
                        logInfo.is_matched = 1;
                        logInfo.rule_id = pRule.id;
                        addMatchedRule(pRule.id);
                        this->_processedRules.push_back(i);
                    }
                }
            }

            if (!ruleInfo.regex.empty()) /* Checking the regex patterns if exists in the rule */
            {
                if ((result = regexMatch(logInfo.log, ruleInfo.regex)) == SUCCESS)
                {
                    // logInfo.log += "|" + std::to_string(ruleInfo.level);
                    // logInfo.log += "|" + std::to_string(ruleInfo.id);
                    logInfo.is_matched = 1;
                    logInfo.rule_id = ruleInfo.id;
                    addMatchedRule(ruleInfo.id);
                }
            }

            if (ruleInfo.max_log_size > 0) /* Validating the syslog size */
            {
                if ((result = isValidSysLog(logInfo.size)) == FAILED)
                {
                    logInfo.is_matched = 1;
                    logInfo.rule_id = ruleInfo.id;
                    addMatchedRule(ruleInfo.id);
                }
            }
            
            if (ruleInfo.if_sid > 0)
            {
                if (isRuleFound(ruleInfo.if_sid))
                {
                    logInfo.is_matched = 1;
                    logInfo.rule_id = ruleInfo.id; // Adding alerted log to the alert file.
                    addMatchedRule(ruleInfo.id);
                }
            }
            
            if (ruleInfo.frequency > 0 && ruleInfo.timeframe > 0) /*Started the particular log files.*/
            {
                pRule.id        = ruleInfo.id;
                pRule.start     = AgentUtils::convertStrToTime(logInfo.timestamp);
                pRule.end       = pRule.start + ruleInfo.timeframe;
                pRule.frequency = ruleInfo.frequency;   
                pRule.d_frequency = 0;
                pRule.time      = ruleInfo.timeframe;
                if (ruleInfo.same_source_ip == 1)
                {
                    pRule.same_source_ip = 1;
                }
            }

            if (ruleInfo.if_sid == 1)
            {
                pRule.if_sid = ruleInfo.if_sid;
                pRule.src_ip   = logInfo.src_ip;
                if (isRuleFound(pRule.if_sid))
                {
                    pRule.frequency--;
                }
            }   

            if (!logInfo.src_ip.empty() && this->_processingRules.size() > 0)
            {
                for (P_RULE p: this->_processingRules)
                {
                    if (strcmp(p.src_ip.c_str(), logInfo.src_ip.c_str()) == 0 && p.same_source_ip == 1)
                    {
                        p.d_frequency++;
                    }
                    else
                    {
                        p.src_ip = logInfo.src_ip;
                        p.start  = AgentUtils::convertStrToTime(logInfo.timestamp);
                        p.end    = p.start + p.time;
                        p.d_frequency = 1;
                    }
                }
            }

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
    vector<LOG_EVENT> alertLogs;
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
        if (logInfo.is_matched == 1)
        {
            alertLogs.push_back(logInfo);
        }
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