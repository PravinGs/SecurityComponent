#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19

#include "service/configservice.hpp"

typedef struct P_RULE P_RULE;

struct P_RULE
{
    int id;
    int if_sid;
    int same_source_ip;
    int time;
    std::time_t start;
    std::time_t end;
    int frequency;
    int d_frequency;
    string src_ip;
};

class LogAnalysis
{
private:
    string _rulesFile;
    IniConfig _configService;
    vector<P_RULE> _processingRules;
    vector<int> _processedRules;
    vector<int> _idRules;

private:
    bool isRuleFound(const int ruleId);
    void addMatchedRule(const int ruleId, const string log);

public:

    LogAnalysis();
    
    LogAnalysis(const string configFile);
    
    void setConfigFile(const string configFile);

    int isValidSysLog(size_t size);

    LOG_EVENT parseToLogInfo(string log, const string format);

    string formatSysLog(string log, const string format);

    bool regexMatch(const string log, const string pattern);

    bool pcreMatch(const string input, const string pattern);
    
    int match(LOG_EVENT &logInfo,map<string, map<int, AConfig>> rules);

    int analyseFile(const string file, const string format);

    int start(const string path);

    ~LogAnalysis(){}
};

#endif