#include "service/loganalysis.hpp"


const string AFTER_PARENT = "after_parent";
const string AFTER_PREMATCH = "after_prematch";
const string AFTER_PCRE2 = "after_pcre2";

LogAnalysis::LogAnalysis() {}

void LogAnalysis::setConfigFile(const string &decoderPath, const string &ruledDir)
{

    int result = _configService.readDecoderConfig(decoderPath, _decoder_list);
    if (result == FAILED)
    {
        isValidConfig = false;
    }
    this->_rulesFile = ruledDir;
    result = _configService.readXmlRuleConfig(ruledDir, _rules);
    if (result == FAILED)
    {
        isValidConfig = false;
    }
}

void extractNetworkLog(log_event &logInfo)
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

int LogAnalysis::isValidSysLog(size_t size)
{
    return (size <= OS_SIZE_1024) ? SUCCESS : FAILED;
}

string LogAnalysis::decodeGroup(log_event & logEvent)
{
    string group;
    
    for (const auto &d: this->_decoder_list)
    {
        decoder p = d.second;
        int match = 0;
        string match_data;
        string after_match;
        size_t position = 0;

        if (!p.program_name_pcre2.empty())
        {               
            match = pcreMatch(logEvent.program, p.program_name_pcre2, match_data, position);
            after_match = logEvent.program.substr(position);
            if (match == 1)
            {
                group = (!p.parent.empty()) ? p.parent : p.decoder;
                break;
            }
        }
        
        if (!p.prematch_pcre2.empty())   
        {
            string input = (!p.prematch_offset.empty() && p.prematch_offset == AFTER_PARENT) ? logEvent.message : logEvent.log;  

            match = pcreMatch(input, p.prematch_pcre2, match_data, position);
            after_match = input.substr(position);
            if (match == 1)
            {
                group = (!p.parent.empty()) ? p.parent : p.decoder;
                break;
            }
        }

        if (!p.pcre2.empty())   
        {
            string input;
            
            if (p.pcre2_offset == AFTER_PARENT)
            {
                input = logEvent.message;
            }
            else if (p.pcre2_offset == AFTER_PREMATCH)
            {
                input = (match_data.empty()) ? logEvent.log : match_data; /*Need to update*/
            } 
            else
            {
                input = logEvent.log;
            }
            match = pcreMatch(input, p.pcre2, match_data, position);
            after_match = input.substr(position);
            if (match == 1)
            {
                group = (!p.parent.empty()) ? p.parent : p.decoder;
                break;
            }
        }
        
    }
    return group;
}

log_event LogAnalysis::decodeLog(const string &log, const string &format)
{
    log_event logInfo;
    string timestamp, user, program, message, group;

    const string &logToParse = (std::count(log.begin(), log.end(), '|') >= 2) ? log : formatSysLog(log, format);

    if (logToParse.empty())
        return logInfo;

    std::istringstream ss(logToParse);

    std::getline(ss, timestamp, '|');
    std::getline(ss, user, '|');
    std::getline(ss, program, '|');
    std::getline(ss, message, '|');
    logInfo.log = log;
    logInfo.format = (strcmp(format.c_str(), "auth") == 0) ? "pam" : format;
    logInfo.size = log.size();
    logInfo.timestamp = timestamp;
    logInfo.user = user;
    logInfo.program = program;
    logInfo.message = message;
    decodeGroup(logInfo); // Need to invoke the decoder group function

    if (logInfo.log.find("SRC=") != string::npos)
    {
        extractNetworkLog(logInfo);
    }
    return logInfo;
}

string LogAnalysis::formatSysLog(const string &log, const string &format)
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

int LogAnalysis::regexMatch(const string &log, const string &pattern, string & match)
{
    std::regex r(pattern);
    std::smatch matches;
    int result = regex_search(log, matches, r);
    for (const auto& s: matches)
    {
        match += s.str();
    }
    return (result && matches.size() > 0) ? 1 : 0;
}

int LogAnalysis::pcreMatch(const string &input, const string &pattern, string& match, size_t & position)
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
        std::cerr << "PCRE2 compilation failed at offset " << erroroffset << ": " << buffer << "\n";
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
    
    if (rc >= 0) {
         PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);

        // Extract the matched substring
        const char *matched_string_start = reinterpret_cast<const char*>(input.c_str()) + ovector[0];
        PCRE2_SIZE matched_string_length = ovector[1] - ovector[0];

        std::string matched_str(matched_string_start, matched_string_length);
        
        match = matched_str;

        // Calculate the position of the last matched value
        PCRE2_SIZE last_match_start = ovector[2 * (rc - 1)]; // Start offset of the last match
        PCRE2_SIZE last_match_length = ovector[2 * (rc - 1) + 1] - ovector[2 * (rc - 1)]; // Length of the last match

        // Calculate the end position of the last matched value
        PCRE2_SIZE last_match_end = last_match_start + last_match_length;
        position = reinterpret_cast<size_t>(last_match_end);
        //match = input.substr(position);
        // Now, 'matched_str' contains the matched substring as a C++ std::string
    }
    
    
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

void LogAnalysis::addMatchedRule(const int ruleId, const string &log)
{
    int result = isRuleFound(ruleId);
    if (result == FAILED)
    {
        {
            this->_idRules.push_back(ruleId);
        }
        AgentUtils::writeLog("Rule Id: " + std::to_string(ruleId) + " -> " + log, DEBUG);
    }
    return;
}

int LogAnalysis::match(log_event &logInfo, std::unordered_map<int, AConfig>& ruleSet)
{
    for (const auto &r : ruleSet)
    {
        bool isParentRuleMatching = false;
        string match_data;
        size_t position;
        AConfig ruleInfo = r.second;
        p_rule pRule;
        {
            std::set<int> processedRuleIDs;
            for (int i : this->_processedRules)
            {
                processedRuleIDs.insert(i);
            }
            _processingRules.erase(
                std::remove_if(
                    _processingRules.begin(),
                    _processingRules.end(),
                    [&processedRuleIDs](const p_rule &s)
                    {
                        // Return true if the p_rule.id is in processedRuleIDs
                        return processedRuleIDs.find(s.id) != processedRuleIDs.end();
                    }),
                _processingRules.end());
        }

        if (!ruleInfo.regex.empty()) /* Checking the regex patterns if exists in the rule */
        {
            int result = regexMatch(logInfo.log, ruleInfo.regex, match_data);
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

        if (ruleInfo.pcre2.size() > 0)
        {
            int result;
            for (const string& pattern: ruleInfo.pcre2)
            {
                result = pcreMatch(logInfo.log, pattern, match_data, position);
                if (result > 0 && !match_data.empty())
                {
                    logInfo.is_matched = 1;
                    logInfo.rule_id = ruleInfo.id;
                    logInfo.group = ruleInfo.group;
                    isParentRuleMatching = true;
                    addMatchedRule(ruleInfo.id, logInfo.log);
                    break;
                }
            }
            if (result > 0 && result > 0 && !match_data.empty())
            {
                break;   
            }
            
        }

        if (!ruleInfo.program_name_pcre2.empty())
        {
            int result = pcreMatch(logInfo.program, ruleInfo.program_name_pcre2, match_data, position);
            if (result > 0 && !match_data.empty())
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
            // pRule.d_frequency = 1;
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
            for (p_rule p : this->_processingRules)
            {
                if (isRuleFound(p.if_mid) == SUCCESS)
                {
                    p.d_frequency++;
                }
            }
        }

        if (ruleInfo.same_source_ip == 1 && !logInfo.src_ip.empty()) /* Is is firewall related log. Then check any rules are being in processing state.*/
        {
            for (p_rule p : this->_processingRules) /*Processing rule has the src_ip, that frequency will be monitored for the given timeframe in the rule.*/
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
            for (p_rule p : this->_processingRules)
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
                p_rule pRule = this->_processingRules[i];
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

int LogAnalysis::match(log_event &logInfo)
{
    std::unordered_map<int, AConfig> currentRuleSet;
    int result = SUCCESS;
    if (logInfo.format.empty() || this->_rules.find(logInfo.group) == this->_rules.end())
    {
        for (auto &r : this->_rules)
        {
            currentRuleSet = r.second;
            result = match(logInfo, currentRuleSet);
        }
    }
    else
    {
       result = match(logInfo, this->_rules.at(logInfo.group));
    }
    return result;
}

int LogAnalysis::analyseFile(const string &file)
{
    string line, format;
    vector<log_event> alertLogs;
    fstream fp(file, std::ios::in);
    if (!fp)
    {
        AgentUtils::writeLog(FILE_ERROR + file, FAILED);
        return FAILED;
    }
    if (!isValidConfig) // Validating rules are extracted or not.
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
        log_event logInfo = decodeLog(line, format);
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

int LogAnalysis::start(const string &decoderPath, const string &rulesDir, const string &path)
{
    int result = SUCCESS;
    string format;
    vector<string> files;

    // Setting configuration files for the LogAnalysis instance;
    {
        setConfigFile(decoderPath, rulesDir);
    }

    if (!isValidConfig)
        return FAILED;

    // Actual start function implentation

    int isFile = (OS::isDirExist(path) && std::filesystem::is_regular_file(path)) ? 1 : 0;

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
        for (const string &file : files)
        {
            result = analyseFile(file);
        }
    }
    return result;
}

int LogAnalysis::postAnalysis(const vector<log_event> &alerts)
{
    for (const auto &log : alerts)
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

int LogAnalysis::printLogDetails(const AConfig &ruleInfo, const log_event &logInfo)
{
    AConfig child = ruleInfo;
    cout << "Timestamp : " << logInfo.timestamp << "\n";
    cout << "user      : " << logInfo.user << "\n";
    cout << "program   : " << logInfo.program << "\n";
    cout << "log       : " << logInfo.log << "\n";
    cout << "rule      : " << child.id << "\n";
    while (child.if_sid > 0)
    {
        child = getRule(logInfo.group, child.if_sid);
    }
    if (ruleInfo.description.empty())
    {
        cout << "category  : " << child.decoded_as << "\n";
    }
    else
    {
        cout << "category  : " << ruleInfo.decoded_as << "\n";
    }

    if (ruleInfo.description.empty())
    {
        cout << "Description: " << child.description << "\n";
    }
    else
    {
        cout << "Description: " << ruleInfo.description << "\n";
    }
    return SUCCESS;
}

AConfig LogAnalysis::getRule(const string &group, const int ruleId)
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
