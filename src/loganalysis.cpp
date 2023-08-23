#include "service/loganalysis.hpp"

LogAnalysis::LogAnalysis(const string configFile) : _rulesFile(configFile)
{
    int result = _configService.readRuleConfig(_rulesFile, this->_rules);
    if (result == SUCCESS)
    {
        isValidConfig = true;
    }
}

LogAnalysis::LogAnalysis() {}

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

    logInfo.src_ip = src_ip;
    logInfo.dest_ip = dest_ip;
    logInfo.proto = proto;

    return;
}

void LogAnalysis::setConfigFile(const string configFile)
{
    this->_rulesFile = configFile;
    int result = _configService.readRuleConfig(_rulesFile, this->_rules);
    if (result == SUCCESS)
    {
        isValidConfig = true;
    }
}

int LogAnalysis::isValidSysLog(size_t size)
{
    return (size <= OS_SIZE_1024) ? SUCCESS : FAILED;
}

LOG_EVENT LogAnalysis::parseToLogInfo(string log, const string format)
{
    LOG_EVENT logInfo;
    string timestamp, user, program, message;
    std::stringstream ss;
    if (std::count(log.begin(), log.end(), '|') >= 2)
    {
        ss << log;
    }
    else
    {
        string fLog = formatSysLog(log, format);
        if (fLog.empty())
        {
            return logInfo;
        }
        ss << fLog;
    }
    std::getline(ss, timestamp, '|');
    std::getline(ss, user, '|');
    std::getline(ss, program, '|');
    std::getline(ss, message, '|');
    logInfo.format = (strcmp(format.c_str(), "auth") == 0) ? "pam" : format;
    logInfo.size = log.size();
    logInfo.timestamp = timestamp;
    logInfo.user = user;
    logInfo.program = program;
    logInfo.log = AgentUtils::trim(message);

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
    return ((regex_search(log, matches, r) && matches.size() > 0)) ? 1 : 0;
}

int LogAnalysis::pcreMatch(const std::string &input, const std::string &pattern)
{
    int errorcode = -1;
    PCRE2_SIZE erroroffset;

    // Compile the pattern
    pcre2_code *re = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR8>(pattern.c_str()), // Pattern string
        PCRE2_ZERO_TERMINATED,                          // Length of pattern
        PCRE2_CASELESS,                                 // Compile options (case-insensitive)
        &errorcode,                                     // Error code
        &erroroffset,                                   // Error offset
        nullptr);                                       // Compile context

    if (re == nullptr)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        std::cerr << "PCRE2 compilation failed at offset " << erroroffset << ": " << buffer << std::endl;
        return FAILED;
    }

    // Match the input against the pattern
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, nullptr);
    int rc = pcre2_match(
        re,                                           // Compiled pattern
        reinterpret_cast<PCRE2_SPTR8>(input.c_str()), // Input string
        PCRE2_ZERO_TERMINATED,                        // Length of input
        0,                                            // Start offset
        0,                                            // Match options
        match_data,                                   // Match data
        nullptr);                                     // Match context

    pcre2_code_free(re);
    pcre2_match_data_free(match_data);

    return rc;
}

int LogAnalysis::isRuleFound(const int ruleId)
{
    // auto result = std::find(_idRules.begin(), _idRules.end(), ruleId);
    // return (result == _idRules.end()) ? FAILED : SUCCESS;
    for (int rule : _idRules)
    {
        if (rule == ruleId)
            return SUCCESS;
    }
    return FAILED;
}

void LogAnalysis::addMatchedRule(const int ruleId, const string log)
{
    int result = isRuleFound(ruleId);
    if (result == FAILED)
    {
        this->_idRules.push_back(ruleId);
        AgentUtils::writeLog("Rule Id: " + std::to_string(ruleId) + " -> " + log, DEBUG);
    }
    return;
}

int LogAnalysis::match(LOG_EVENT &logInfo)
{
    if (logInfo.format.empty() || this->_rules.find(logInfo.format) == this->_rules.end())
    {
        return FAILED;
    }
    map<int, AConfig> currentRuleSet = this->_rules.at(logInfo.format);
    for (const auto &r : currentRuleSet)
    {
        bool isParentRuleMatching = false;
        AConfig ruleInfo = r.second;
        P_RULE pRule;

        for (int i : this->_processedRules) /* Removing all the processed rules. */
        {
            auto newEnd = std::remove_if(_processingRules.begin(), _processingRules.end(),
                                         [i](const P_RULE &s)
                                         {
                                             return s.id == i;
                                         });
            this->_processingRules.erase(newEnd, _processingRules.end());
        }

        if (!ruleInfo.regex.empty()) /* Checking the regex patterns if exists in the rule */
        {
            int result = regexMatch(logInfo.log, ruleInfo.regex);
            if (result == 1)
            {
                logInfo.is_matched = 1;
                logInfo.rule_id = ruleInfo.id;
                logInfo.group = ruleInfo.group;
                isParentRuleMatching = true;
                addMatchedRule(ruleInfo.id, logInfo.log);
                break;
            }
        }

        if (!ruleInfo.pcre2.empty())
        {
            int result = pcreMatch(logInfo.log, ruleInfo.pcre2);
            if (result > 0)
            {
                logInfo.is_matched = 1;
                logInfo.rule_id = ruleInfo.id;
                logInfo.group = ruleInfo.group;
                isParentRuleMatching = true;
                addMatchedRule(ruleInfo.id, logInfo.log);
                break;
            }
        }

        if (!ruleInfo.program_name_pcre2.empty())
        {
            int result = pcreMatch(logInfo.log, ruleInfo.pcre2);
            if (result > 0)
            {
                logInfo.is_matched = 1;
                logInfo.rule_id = ruleInfo.id;
                logInfo.group = ruleInfo.group;
                isParentRuleMatching = true;
                addMatchedRule(ruleInfo.id, logInfo.log);
                break;
            }
        }

        if (ruleInfo.max_log_size > 0) /* Validating the syslog size */
        {
            int result = isValidSysLog(logInfo.size);
            if (result == FAILED)
            {
                logInfo.is_matched = 1;
                logInfo.rule_id = ruleInfo.id;
                logInfo.group = ruleInfo.group;
                addMatchedRule(ruleInfo.id, logInfo.log);
                break;
            }
        }

        if (ruleInfo.frequency > 0 && ruleInfo.timeframe > 0) /*Check if the rule has time based matcher, if it is mark this rule as the processing rule.*/
        {
            pRule.id = ruleInfo.id;
            pRule.start = AgentUtils::convertStrToTime(logInfo.timestamp);
            pRule.end = pRule.start + ruleInfo.timeframe;
            pRule.frequency = ruleInfo.frequency;
            pRule.d_frequency = 1;
            pRule.time = ruleInfo.timeframe;
            if (ruleInfo.same_source_ip == 1) /* Check at the XML pasing. */
            {
                pRule.same_source_ip = 1;
            }
            if (ruleInfo.same_id == 1)
            {
                pRule.same_id = 1;
            }
        }

        if (ruleInfo.if_matched_id > 0) /* If matched_sid enabled apply the child rules alert value to this rule. */
        {
            pRule.if_mid = ruleInfo.if_matched_id;
            if (isRuleFound(ruleInfo.if_matched_id) == SUCCESS)
            {
                pRule.d_frequency++;
                logInfo.is_matched = 1;
                logInfo.rule_id = ruleInfo.id;
                logInfo.group = ruleInfo.group;
                addMatchedRule(ruleInfo.id, logInfo.log);
                break;
            }
        }
        else if (ruleInfo.if_matched_id > 0 && ruleInfo.same_id == 1)
        {
            pRule.if_mid = ruleInfo.if_matched_id;
            for (P_RULE p : this->_processingRules)
            {
                if (isRuleFound(p.if_mid) == SUCCESS)
                {
                    p.d_frequency++;
                }
            }
        }

        if (ruleInfo.same_source_ip == 1 && !logInfo.src_ip.empty()) /* Is is firewall related log. Then check any rules are being in processing state.*/
        {
            for (P_RULE p : this->_processingRules) /*Processing rule has the src_ip, that frequency will be monitored for the given timeframe in the rule.*/
            {
                if (strcmp(p.src_ip.c_str(), logInfo.src_ip.c_str()) == 0 && p.same_source_ip == 1) /*src_ip matching, if it matches remove increase the helper_varible.*/
                {
                    p.d_frequency++;
                }
            }
        }

        if (ruleInfo.if_sid > 0 && ruleInfo.same_id == 1) /*If the child id of this rule exists, then you can set child id to the log*/
        {
            pRule.if_sid = ruleInfo.if_sid;
            for (P_RULE p : this->_processingRules)
            {
                if (isRuleFound(p.if_sid == SUCCESS))
                {
                    p.d_frequency++;
                }
            }
        }

        if (isParentRuleMatching)
        {
            logInfo.rule_id = ruleInfo.id;
        } /* If the parent, child matched, priority to parent. */

        if (pRule.id > 0)
        {
            this->_processingRules.push_back(pRule);
        }

        if (this->_processingRules.size() > 0) /* Checking if rules are about to expire or not. */
        {
            for (int i = 0; i < (int)this->_processingRules.size(); i++)
            {
                P_RULE pRule = this->_processingRules[i];
                if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.d_frequency != pRule.frequency))
                {
                    this->_processedRules.push_back(i); /* The rule expired */
                }
                else if ((pRule.end < AgentUtils::convertStrToTime(logInfo.timestamp) && pRule.frequency == pRule.d_frequency) || pRule.frequency <= pRule.d_frequency)
                {
                    /*The rule ID'd and have to alerted*/
                    logInfo.is_matched = 1;
                    logInfo.rule_id = pRule.id;
                    logInfo.group = pRule.group;
                    addMatchedRule(pRule.id, logInfo.log);
                    this->_processedRules.push_back(i);
                    break;
                }
            }
        }
    }
    return SUCCESS;
}

int LogAnalysis::analyseFile(const string file)
{
    string line, format;
    vector<LOG_EVENT> alertLogs;
    fstream fp(file, std::ios::in);
    if (!fp)
    {
        AgentUtils::writeLog(FILE_ERROR + file, FAILED);
        return FAILED;
    }
    if (!isValidConfig) /*Validating rules are extracted or not.*/
    {
        AgentUtils::writeLog("Failed to parse XML configuration file, check the file", FAILED);
        fp.close();
        return FAILED;
    }
    if (file.find("dpkg") != string::npos)
    {
        format = "dpkg";
    }
    else if (file.find("auth") != string::npos)
    {
        format = "auth";
    }
    else
    {
        format = "syslog";
    }
    AgentUtils::writeLog("Log analysis started for " + file, INFO);
    while (std::getline(fp, line))
    {
        if (line.empty())
        {
            continue;
        }
        LOG_EVENT logInfo = parseToLogInfo(line, format);
        match(logInfo);
        if (logInfo.is_matched == 1)
        {
            alertLogs.push_back(logInfo);
        }
    }
    AgentUtils::writeLog("Total matched logs : " + std::to_string(alertLogs.size()), DEBUG);
    fp.close();
    return postAnalysis(alertLogs);
}

int LogAnalysis::start(const string path)
{
    string format;
    int result = SUCCESS;
    int isFile = 0;
    vector<string> files;

    if (OS::isDirExist(path) && std::filesystem::is_regular_file(path))
    {
        isFile = 1;
    }

    if (isFile)
    {
        result = analyseFile(path);
    }
    else
    {
        string currentFile = path;
        if (currentFile[currentFile.size() - 1] == '/')
        {
            currentFile = currentFile.substr(0, currentFile.find_last_of('/'));
        }
        result = OS::getRegularFiles(currentFile, files);

        if (result == FAILED)
            return FAILED;

        if (files.size() == 0)
        {
            AgentUtils::writeLog(INVALID_PATH + path, FAILED);
            return FAILED;
        }
        for (string file : files)
        {
            result = analyseFile(file);
        }
    }
    return result;
}

int LogAnalysis::postAnalysis(const vector<LOG_EVENT> alerts)
{
    for (LOG_EVENT log : alerts)
    {
        AConfig config = getRule(log.group, log.rule_id);
        if (config.id <= 0)
        {
            AgentUtils::writeLog("Unrecognized rule, Ruleid=" + std::to_string(log.rule_id) + " RuleGroup=" + log.group, WARNING);
        }
        else
        {
            printLogDetails(config, log);
        }
    }
    return SUCCESS;
}

int LogAnalysis::printLogDetails(AConfig ruleInfo, LOG_EVENT logInfo)
{
    AConfig child = ruleInfo;
    cout << "Timestamp : " << logInfo.timestamp << endl;
    cout << "user      : " << logInfo.user << endl;
    cout << "program   : " << logInfo.program << endl;
    cout << "log       : " << logInfo.log << endl;
    cout << "rule      : " << child.id << endl;
    while (child.if_sid > 0)
    {
        child = getRule(logInfo.group, child.if_sid);
    }
    if (ruleInfo.description.empty())
    {
        cout << "category  : " << child.decoded_as << endl;
    }
    else
    {
        cout << "category  : " << ruleInfo.decoded_as << endl;
    }

    if (ruleInfo.description.empty())
    {
        cout << "Description: " << child.description << endl;
    }
    else
    {
        cout << "Description: " << ruleInfo.description << endl;
    }
    return SUCCESS;
}

AConfig LogAnalysis::getRule(const string group, const int ruleId)
{
    AConfig config;
    for (const auto &parent : _rules)
    {
        for (const auto &child : parent.second)
        {
            if (child.first == ruleId)
            {
                config = child.second;
                break;
            }
        }
    }
    return config;
}

string LogAnalysis::formatSysLog(string log, const string format)
{
    string fLog, token, formattedTime;
    int index = 0;
    string currentTime = log.substr(0, 15);
    std::stringstream stream;
    if (format == "dpkg")
    {
        string temp, host;
        AgentUtils::getHostName(host);
        fLog += log.substr(0, 19);
        fLog += "|" + host;
        temp = log.substr(20);
        fLog += "|" + temp.substr(0, temp.find(' '));
        temp = temp.substr(temp.find(' ') + 1);
        fLog += "|" + temp;
        return fLog;
    }
    else
    {
        if (AgentUtils::convertTimeFormat(currentTime, formattedTime) == FAILED)
        {
            return "";
        }
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

    return fLog;
}