#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19

#include "agentUtils.hpp"
#include "service/configservice.hpp"

typedef P_RULE P_RULE;

struct P_RULE
{
    int id;
    int child_id;
    std::time_t start;
    std::time_t end;
    int frequency;
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
    void addMatchedRule(const int ruleId);

public:

    LogAnalysis();
    
    LogAnalysis(const string configFile);
    
    void setConfigFile(const string configFile);

    int isValidSysLog(size_t size);

    LOG_EVENT parseToLogInfo(string log, const string format);

    string formatSysLog(string log, const string format);

    int regexMatch(const string log, const string pattern);
    
    int match(LOG_EVENT &logInfo,map<string, map<int, AConfig>> rules);

    int getRegularFiles(const string directory, vector<string> &files);

    int analyseFile(const string file, const string format);

    int start(const string path);

    ~LogAnalysis(){}
};

#endif