#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19

#include "service/configservice.hpp"

typedef struct P_RULE P_RULE;

struct P_RULE
{
    int id;
    int if_sid;
    int if_mid;
    int same_source_ip;
    int same_id;
    int time;
    std::time_t start;
    std::time_t end;
    int frequency;
    int d_frequency;
    string src_ip;
    string group;

    P_RULE() : id(0), if_sid(0), if_mid(0), same_source_ip(0),
               same_id(0), time(0), frequency(0), d_frequency(0) {}
};

class LogAnalysis
{
private:
    string _rulesFile;
    IniConfig _configService;
    vector<P_RULE> _processingRules;
    vector<int> _processedRules;
    vector<int> _idRules;
    map<string, map<int, AConfig>> _rules;

private:
    bool isValidConfig = false;
    int isRuleFound(const int ruleId);
    void addMatchedRule(const int ruleId, const string log);

public:
    LogAnalysis();
    
    LogAnalysis(const string configFile);
    
    void setConfigFile(const string configFile);

    int isValidSysLog(size_t size);

    LOG_EVENT parseToLogInfo(string log, const string format);

    string formatSysLog(string log, const string format);

    int regexMatch(const string log, const string pattern);

    int pcreMatch(const std::string& input, const std::string& pattern);
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
    void match(LOG_EVENT &logInfo);

    int analyseFile(const string file, const string format);

    int start(const string path);

    int postAnalysis(const vector<LOG_EVENT> alerts);

    AConfig getRule(const string group,const int ruleId);

    int printLogDetails(AConfig ruleInfo, LOG_EVENT logInfo);


    ~LogAnalysis(){}
};

#endif