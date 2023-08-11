#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19

#include "agentUtils.hpp"

class LogAnalysis
{
private:
    string _format = "syslog";
public:

    LogAnalysis(){}
    LogAnalysis(const string format) : _format(format) {}
    
    int isValidSysLog(size_t size)
    {
        return (size <= OS_SIZE_1024) ? SUCCESS : FAILED;
    }

    LOG_EVENT parseToLogInfo(string log)
    {
        /* Log format should be checked before parsing */
        LOG_EVENT logInfo;
        string timestamp, user, program, message;
        std::stringstream ss(log);
        std::getline(ss, timestamp, '|');
        std::getline(ss, user, '|');
        std::getline(ss, program, '|');
        std::getline(ss, message, '|');
        /* Set the values to the structure */
        logInfo.size = log.size();
        logInfo.format = _format; 
        logInfo.timestamp = timestamp;
        logInfo.user = user;
        logInfo.program = program;
        logInfo.log = message;
        return logInfo;
    }

    int getRegularFiles(const string directory, vector<string> &files)
    {
        int result = SUCCESS;
        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(directory))
            {
                if (std::filesystem::is_regular_file(entry.path()))
                {
                    files.push_back(entry.path());
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

    void printLogInfo(LOG_EVENT logInfo)
    {
        cout << "size : " << logInfo.size << endl;
        cout << "log : " << logInfo.log << endl;
        cout << "format : " << logInfo.format << endl;
        cout << "timestamp : " << logInfo.timestamp << endl;
        cout << "program : " << logInfo.program << endl;
        cout << "user : " << logInfo.user << endl;   
    }

    int analyseFile(const string file)
    {
        fstream fp(file, std::ios::in);
        if (!fp)
        {
            AgentUtils::writeLog("Invalid file path given for analysis ( " + file + " )", FAILED);
            return FAILED;
        }
        string line;

        while(std::getline(fp, line))
        {
            if (line.empty()) continue;
            LOG_EVENT logInfo = parseToLogInfo(line);
            printLogInfo(logInfo);
            cout << "*****************" << endl;
        }
        
        fp.close();
        return SUCCESS;
    }

    int start(const string path)
    {

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
            result = getRegularFiles(path, files);
            if (result == FAILED) return FAILED;
            string currentFile = path;
            if (currentFile[currentFile.size() - 1] != '/')
            {
                currentFile += "/";
            }
            if (files.size() == 0)
            {
                AgentUtils::writeLog("Check directory ( " + path + " ). It contains no files.", FAILED);
                return FAILED;
            }
            for (string file : files)
            {
                currentFile += file;
                result = analyseFile(currentFile);
            }
        }
        return result;
    }

    ~LogAnalysis(){}
};

#endif