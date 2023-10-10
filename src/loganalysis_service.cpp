#include "service/loganalysis_service.hpp"

const string AFTER_PARENT = "after_parent";
const string AFTER_PREMATCH = "after_prematch";
const string AFTER_PCRE2 = "after_pcre2";

log_analysis::log_analysis() {}

void log_analysis::set_config_file(const string &decoderPath, const string &rule_path)
{
    int result = _config_service.read_decoder_config(decoderPath, _decoder_list);
    if (result == FAILED)
    {
        _is_valid_config = false;
    }
    result = _config_service.read_xml_rule_config(rule_path, _rules);
    if (result == FAILED)
    {
        _is_valid_config = false;
    }
}

void extract_network_log(log_event &log_info)
{
    /* IN= OUT=lo SRC=127.0.0.1 DST=127.0.0.1 LEN=52 TOS=0x00 PREC=0x00
    TTL=64 ID=38860 DF PROTO=TCP SPT=8888 DPT=55764 WINDOW=512 RES=0x00 ACK URGP=0 */

    string temp = log_info.log;
    string src_ip = temp.substr(temp.find("SRC") + 4);
    src_ip = src_ip.substr(0, src_ip.find_first_of(' '));
    string dest_ip = temp.substr(temp.find("DST") + 4);
    dest_ip = dest_ip.substr(0, dest_ip.find_first_of(' '));
    string proto = temp.substr(temp.find("PROTO") + 6);
    proto = proto.substr(0, proto.find_first_of(' '));

    log_info.src_ip = src_ip;
    log_info.dest_ip = dest_ip;
    log_info.proto = proto;

    return;
}

int log_analysis::is_valid_syslog(size_t size)
{
    return (size <= OS_SIZE_1024) ? SUCCESS : FAILED;
}

string log_analysis::decode_group(log_event & log_info)
{
    string group;
    // cout << "size : " << _decoder_list.size() << "\n";
    
    for (const auto &d: _decoder_list)
    {
        decoder p = d.second;
        // cout << log_info.program << " = " << p.decode << "\n";
        int match = 0;
        string match_data;
        string after_match;
        size_t position = 0;

        if (!p.program_name_pcre2.empty())
        {               
            match = pcre_match(log_info.program, p.program_name_pcre2, match_data, position);
            after_match = log_info.program.substr(position);
            if (match == 1 && position > 0)
            {
                group = (!p.parent.empty()) ? p.parent : p.decode;
                add_decoder_cache(group);
                // cout << group << " (program name pcre2)\n";
                // cout << p.decode << " (program name pcre2)\n";
                break;

            }
        }
        
        if (!p.prematch_pcre2.empty())   
        {
            string input = (!p.prematch_offset.empty() && p.prematch_offset == AFTER_PARENT) ? log_info.message : log_info.log;  

            match = pcre_match(input, p.prematch_pcre2, match_data, position);
            after_match = input.substr(position);
            if (match == 1 && position > 0)
            {    // cout << "size : " << _decoder_list.size() << "\n";

                group = (!p.parent.empty()) ? p.parent : p.decode;
                add_decoder_cache(group);
                // cout << group << " (prematch)\n";
                // cout << p.decode << " (prematch)\n";
                break;
            }
        }

        if (!p.pcre2.empty())   
        {
            string input;
            
            if (p.pcre2_offset == AFTER_PARENT)
            {
                input = log_info.message;
            }
            else if (p.pcre2_offset == AFTER_PREMATCH)
            {
                input = (match_data.empty()) ? log_info.log : match_data; /*Need to update*/
            } 
            else
            {
                input = log_info.log;
            }
            match = pcre_match(input, p.pcre2, match_data, position);
            after_match = input.substr(position);
            if (match == 1 && position > 0)
            {
                group = (!p.parent.empty()) ? p.parent : p.decode;
                add_decoder_cache(group);
                break;
                
            }
        } 
    }
    return group;
}

void log_analysis::add_decoder_cache(const string & decoder)
{
    if (_decoder_cache.size() >= MAX_CACHE_SIZE)
    {
        _decoder_cache.insert(_decoder_cache.begin() + 0, decoder);
    }
    else
    {
        _decoder_cache.push_back(decoder);
    }
}

log_event log_analysis::decode_log(const string &log, const string &format)
{
    log_event log_info;
    string timestamp, user, program, message, group;

    const string &logToParse = (std::count(log.begin(), log.end(), '|') >= 2) ? log : format_syslog(log, format);

    if (logToParse.empty())
        return log_info;

    std::istringstream ss(logToParse);

    std::getline(ss, timestamp, '|');
    std::getline(ss, user, '|');
    std::getline(ss, program, '|');
    std::getline(ss, message, '|');
    log_info.log = log;
    log_info.format = (strcmp(format.c_str(), "auth") == 0) ? "pam" : format;
    log_info.size = log.size();
    log_info.timestamp = timestamp;
    log_info.user = user;
    log_info.program = (format == "dpkg") ? format : program;
    log_info.message = message;
    log_info.decoded = decode_group(log_info); // Need to invoke the decoder group function

    if (log_info.log.find("SRC=") != string::npos)
    {
        extract_network_log(log_info);
    }
    return log_info;
}

string log_analysis::format_syslog(const string &log, const string &format)
{
    string format_log, token, formatted_time;
    int index = 0;
    string current_time = log.substr(0, 15);
    std::stringstream stream;
    if (format == "dpkg")
    {
        string temp, host;
        agent_utils::get_hostname(host);
        format_log += log.substr(0, 19);
        format_log += "|" + host;
        format_log += "|" + format;
        temp = log.substr(20);
        format_log += "|"+temp;
        return format_log;
    }
    else
    {
        if (agent_utils::convert_time_format(current_time, formatted_time) == FAILED)
        {
            return "";
        }
        stream << log.substr(16);
    }
    format_log += formatted_time;
    while (std::getline(stream, token, ' ') && index < 3)
    {
        if (index == 2)
        {
            string message = token;
            while (std::getline(stream, token, ' '))
            {
                message += ' ' + token;
            }
            format_log += '|' + message;
            index = 4;
            continue;
        }
        format_log += '|' + token;
        index++;
    }

    return format_log;
}

int log_analysis::regex_match(const string &log, const string &pattern, string & match)
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

int log_analysis::pcre_match(const string &input, const string &pattern, string& match, size_t & position)
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

int log_analysis::is_rule_found(const int ruleId)
{
    for (const auto& rule : _matched_rules)
    {
        if (rule.id == ruleId)
            return SUCCESS;
    }
    return FAILED;
}

void log_analysis::add_matched_rule(const id_rule & rule, const string &log)
{
    // cout << "group: " << rule.group << "\tId: " << rule.id << "\n";
    int result = is_rule_found(rule.id);
    if (result == SUCCESS) return;

    if (_matched_rules.size() >= MAX_CACHE_SIZE)
    {
        _matched_rules.insert(_matched_rules.begin() + 0, rule);
    }
    else
    {
        _matched_rules.push_back(rule);
    }
    agent_utils::write_log("Rule Id: " + std::to_string(rule.id) + " -> " + log, DEBUG);
    return;
}

void log_analysis::match(log_event &log_info, std::unordered_map<int, aconfig>& ruleSet)
{
    for (const auto& r: _matched_rules)
    {
        aconfig rule = get_rule(r.group, r.id);
        match(log_info, rule);
        if (log_info.is_matched == 1) break;
    }
    for (auto &r : ruleSet)
    {
        aconfig rule = r.second;
        match(log_info, rule);
        if (log_info.is_matched == 1) break;
    }
    return;
}

void log_analysis::match(log_event & log_info, aconfig & rule_info)
{
    bool is_parent_rule_matching = false;
    string match_data;
    size_t position;
    id_rule matched_rule = { rule_info.group, rule_info.id };
    // cout << matched_rule.group << "\t" << matched_rule.id << "\n";
    p_rule pRule;
    {
        std::set<int> processed_rules_id;
        for (int i : this->_processed_rules)
        {
            processed_rules_id.insert(i);
        }
        _processing_rules.erase(
            std::remove_if(
                _processing_rules.begin(),
                _processing_rules.end(),
                [&processed_rules_id](const p_rule &s)
                {
                    // Return true if the p_rule.id is in processed_rules_id
                    return processed_rules_id.find(s.id) != processed_rules_id.end();
                }),
            _processing_rules.end());
    }

    if (!rule_info.decoded_as.empty() && rule_info.decoded_as == log_info.decoded )
    {
        log_info.is_matched = 1;
        log_info.rule_id = rule_info.id;
        log_info.group = rule_info.group;
        is_parent_rule_matching = true;
        add_matched_rule(matched_rule, log_info.log);
        return;
    }

    if (!rule_info.regex.empty()) /* Checking the regex patterns if exists in the rule */
    {
        int result = regex_match(log_info.log, rule_info.regex, match_data);
        if (result == 1)
        {
            log_info.is_matched = 1;
            log_info.rule_id = rule_info.id;
            log_info.group = rule_info.group;
            is_parent_rule_matching = true;
            add_matched_rule(matched_rule, log_info.log);
            return;
        }
    }

    if (rule_info.pcre2.size() > 0)
    {
        int result;
        for (const string& pattern: rule_info.pcre2)
        {
            result = pcre_match(log_info.log, pattern, match_data, position);
            if (result > 0 && !match_data.empty())
            {
                log_info.is_matched = 1;
                log_info.rule_id = rule_info.id;
                log_info.group = rule_info.group;
                is_parent_rule_matching = true;
                add_matched_rule(matched_rule, log_info.log);
                return;
            }
        }
        if (result > 0 && result > 0 && !match_data.empty())
        {
            return;   
        }
        
    }

    if (!rule_info.program_name_pcre2.empty())
    {
        int result = pcre_match(log_info.program, rule_info.program_name_pcre2, match_data, position);
        if (result > 0 && !match_data.empty())
        {
            log_info.is_matched = 1;
            log_info.rule_id = rule_info.id;
            log_info.group = rule_info.group;
            is_parent_rule_matching = true;
            add_matched_rule(matched_rule, log_info.log);
            return;
        }
    }

    if (rule_info.max_log_size > 0) /* Validating the syslog size */
    {
        int result = is_valid_syslog(log_info.size);
        if (result == FAILED)
        {
            log_info.is_matched = 1;
            log_info.rule_id = rule_info.id;
            log_info.group = rule_info.group;
            add_matched_rule(matched_rule, log_info.log);
            return;
        }
    }

    if (rule_info.frequency > 0 && rule_info.timeframe > 0) /*Check if the rule has time based matcher, if it is mark this rule as the processing rule.*/
    {
        pRule.id = rule_info.id;
        pRule.start = agent_utils::format_string_time(log_info.timestamp);
        pRule.end = pRule.start + rule_info.timeframe;
        pRule.frequency = rule_info.frequency;
        // pRule.d_frequency = 1;
        pRule.time = rule_info.timeframe;
        if (rule_info.same_source_ip == 1) /* Check at the XML pasing. */
        {
            pRule.same_source_ip = 1;
        }
        if (rule_info.same_id == 1)
        {
            pRule.same_id = 1;
        }
    }

    if (rule_info.if_matched_id > 0) /* If matched_sid enabled apply the child rules alert value to this rule. */
    {
        pRule.if_mid = rule_info.if_matched_id;
        if (is_rule_found(rule_info.if_matched_id) == SUCCESS)
        {
            pRule.d_frequency++;
            log_info.is_matched = 1;
            log_info.rule_id = rule_info.id;
            log_info.group = rule_info.group;
            add_matched_rule(matched_rule, log_info.log);
            return;
        }
    }
    else if (rule_info.if_matched_id > 0 && rule_info.same_id == 1)
    {
        pRule.if_mid = rule_info.if_matched_id;
        for (p_rule p : this->_processing_rules)
        {
            if (is_rule_found(p.if_mid) == SUCCESS)
            {
                p.d_frequency++;
            }
        }
    }

    if (rule_info.same_source_ip == 1 && !log_info.src_ip.empty()) /* Is is firewall related log. Then check any rules are being in processing state.*/
    {
        for (p_rule p : this->_processing_rules) /*Processing rule has the src_ip, that frequency will be monitored for the given timeframe in the rule.*/
        {
            if (strcmp(p.src_ip.c_str(), log_info.src_ip.c_str()) == 0 && p.same_source_ip == 1) /*src_ip matching, if it matches remove increase the helper_varible.*/
            {
                p.d_frequency++;
            }
        }
    }

    if (rule_info.if_sid > 0 && rule_info.same_id == 1) /*If the child id of this rule exists, then you can set child id to the log*/
    {
        pRule.if_sid = rule_info.if_sid;
        for (p_rule p : this->_processing_rules)
        {
            if (is_rule_found(p.if_sid == SUCCESS))
            {
                p.d_frequency++;
            }
        }
    }

    if (is_parent_rule_matching)
    {
        log_info.rule_id = rule_info.id;
    } /* If the parent, child matched, priority to parent. */

    if (pRule.id > 0)
    {

        this->_processing_rules.push_back(pRule);
    }

    if (this->_processing_rules.size() > 0) /* Checking if rules are about to expire or not. */
    {
        for (int i = 0; i < (int)this->_processing_rules.size(); i++)
        {
            p_rule pRule = this->_processing_rules[i];
            if ((pRule.end < agent_utils::format_string_time(log_info.timestamp) && pRule.d_frequency != pRule.frequency))
            {
                this->_processed_rules.push_back(i); /* The rule expired */
            }
            else if ((pRule.end < agent_utils::format_string_time(log_info.timestamp) && pRule.frequency == pRule.d_frequency) || pRule.frequency <= pRule.d_frequency)
            {
                /*The rule ID'd and have to alerted*/
                log_info.is_matched = 1;
                log_info.rule_id = pRule.id;
                log_info.group = pRule.group;
                matched_rule.group = pRule.group;
                matched_rule.id = pRule.id;
                add_matched_rule(matched_rule, log_info.log);
                this->_processed_rules.push_back(i);
                return;
            }
        }
    }
    return;
    
}

void log_analysis::match(log_event &log_info)
{
    if (log_info.format.empty() || this->_rules.find(log_info.group) == this->_rules.end())
    {
        for (auto &r : this->_rules)
        {
            match(log_info, r.second);
            if (log_info.is_matched == 1) break;
        }
    }
    else
    {
       match(log_info, this->_rules.at(log_info.group));
    }
}

int log_analysis::analyse_file(const string &file)
{
    string line, format;
    vector<log_event> alert_logs;
    fstream fp(file, std::ios::in);
    if (!fp)
    {
        agent_utils::write_log(FILE_ERROR + file, FAILED);
        return FAILED;
    }
    if (!_is_valid_config) // Validating rules are extracted or not.
    {
        agent_utils::write_log("Failed to parse XML configuration file, check the file", FAILED);
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
    agent_utils::write_log("Log analysis started for " + file, INFO);
    while (std::getline(fp, line))
    {
        if (line.empty())
        {
            continue;
        }
        log_event log_info = decode_log(line, format);
        // cout << "Program : " << log_info.program << "\n";
        // cout << "decoded : " << log_info.decoded << "\n";
        match(log_info);
        if (log_info.is_matched == 1)
        {
            alert_logs.push_back(log_info);
        }
    }
    agent_utils::write_log("Total matched logs : " + std::to_string(alert_logs.size()), DEBUG);
    fp.close();
    return post_log_analysis(alert_logs);
}

int log_analysis::start(const string &decoderPath, const string &rulesDir, const string &path)
{
    int result = SUCCESS;
    string format;
    vector<string> files;

    // Setting configuration files for the log_analysis instance;
    {
        set_config_file(decoderPath, rulesDir);
    }

    if (!_is_valid_config)
        return FAILED;

    // Actual start function implentation

    int isFile = (os::is_dir_exist(path) && std::filesystem::is_regular_file(path)) ? 1 : 0;

    if (isFile)
    {
        result = analyse_file(path);
    }
    else
    {
        string current_file = path;
        if (current_file[current_file.size() - 1] == '/')
        {
            current_file = current_file.substr(0, current_file.find_last_of('/'));
        }
        result = os::get_regular_files(current_file, files);

        if (result == FAILED)
            return FAILED;

        if (files.size() == 0)
        {
            agent_utils::write_log(INVALID_PATH + path, FAILED);
            return FAILED;
        }
        for (const string &file : files)
        {
            result = analyse_file(file);
        }
    }
    return result;
}

int log_analysis::post_log_analysis(const vector<log_event> &alerts)
{
    string host;
    agent_utils::get_hostname(host);
    if (alerts.size() == 0) 
    {
        agent_utils::write_log("No decoder matched", INFO);
        return SUCCESS;
    }
    string filePath = os::get_json_write_path("log-analysis-report");
    Json::Value json;
    json["OrgId"] = 5268;
    json["Source"] = host;
    json["AppName"] = "system_events";
    json["Alerts"] = Json::Value(Json::arrayValue);
    Json::Value alert;
    Json::StreamWriterBuilder writer_builder;

    for (const auto &log : alerts)
    {
        aconfig config = get_rule(log.group, log.rule_id);
        if (config.id <= 0)
        {
            agent_utils::write_log("Unrecognized rule, Ruleid=" + std::to_string(log.rule_id) + " RuleGroup=" + log.group, WARNING);
        }
        else
        {
            // print_log_details(config, log);
            aconfig child = config;
            // string group;
            alert["TimeStamp"]   = log.timestamp;
            alert["User"]        = log.user;
            alert["Program"]     = log.program;
            alert["Message"]     = log.message;
            alert["Category"]    = log.decoded;
            alert["MatchedRule"] = config.id;
            alert["LogLevel"]    = config.level;
            while (child.if_sid > 0)
            {
                child = get_rule(log.group, child.if_sid);
            }

            // group = (config.group.empty()) ? child.group : config.group;

            alert["Section"]  = log.group;
            alert["Description"] = (config.description.empty()) ? child.description : config.description;
            json["Alerts"].append(alert);
        }
    }
    std::ofstream ofile(filePath); /*need update*/
    std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
    writer->write(json, &ofile);
    ofile.close();
    agent_utils::write_log("Log written to " + filePath, SUCCESS);
    return SUCCESS;
}

int log_analysis::print_log_details(const aconfig &rule_info, const log_event &log_info)
{
    aconfig child = rule_info;
    string group = log_info.group;
    cout << "Timestamp : " << log_info.timestamp << "\n";
    cout << "user      : " << log_info.user << "\n";
    cout << "program   : " << log_info.program << "\n";
    cout << "log       : " << log_info.message << "\n";
    cout << "category  : " << log_info.decoded << "\n";
    cout << "rule      : " << child.id << "\n";
    cout << "level     : " << child.level << "\n";
    while (child.if_sid > 0)
    {
        child = get_rule(log_info.group, child.if_sid);
    }
    if (rule_info.description.empty())
    {
        group = child.decoded_as.empty() ? log_info.group : child.decoded_as;
    }
    else
    {
        group = rule_info.decoded_as.empty() ? log_info.group : rule_info.decoded_as;
    }
    cout << "group  : " << group <<"\n";
    if (rule_info.description.empty())
    {
        cout << "Description: " << child.description << "\n";
    }
    else
    {
        cout << "Description: " << rule_info.description << "\n";
    }
    return SUCCESS;
}

aconfig log_analysis::get_rule(const string &group, const int ruleId)
{
    aconfig config;
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
