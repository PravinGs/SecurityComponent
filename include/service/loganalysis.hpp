#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19

#include "agentUtils.hpp"
#include "service/configservice.hpp"

class LogAnalysis
{
private:
    string _rulesFile;
    IniConfig _configService;
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