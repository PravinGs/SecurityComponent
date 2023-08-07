#ifndef AGENT_UTILS_HPP // This header file have all the library includes.
#define AGENT_UTILS_HPP

#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <syslog.h>
#include <jsoncpp/json/json.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ev.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <csignal>
#include <filesystem>
#include <curl/curl.h>
#include <croncpp.h>
#include <boost/asio.hpp>
#include <zlib.h>

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::fstream;
using std::map;
using std::string;
using std::vector;

#define OS_SIZE_1024 1024
#define PATH_MAX 4096
#define MAX_PID 32768
#define PROC_ 0
#define PID  1
#define TASK 2

#define FAILED -1
#define SUCCESS 1
#define WARNING 0

/* Log Levels */
#define NONE 0
#define TRACE 1
#define DEBUG 2
#define WARN 3
#define ERROR 4
#define CRITICAL 5

#define INTERACTION 2
#define STANDARD 3
#define ALARM 4

#define POST_SUCCESS 200L
#define SERVER_ERROR 10
#define BASE_LOG_DIR "/etc/scl/log/"
#define BASE_LOG_ARCHIVE "archives/"
#define BASE_CONFIG_TMP "tmp/"
#define BASE_CONFIG_DIR "/etc/scl/config/"
// #define BASE_CONFIG_DIR "/etc/scl/config/agent/agent.config"

const vector<string> MONTHS = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const int BUFFER_SIZE = 1024;
const int UDP_PORT = 8080;

typedef struct SYS_PROPERTIES SYS_PROPERTIES;

namespace Monitor
{
    struct SYS_PROPERTIES
    {
        double ram;
        double disk;
        double cpu;
    };
    class Data
    {
    private:
        string _processId;
        string _processName;
        string _cpuTime;
        string _memUsage;
        string _diskUsage;

    public:
        Data(string id, string name, string cpuTime, string memUsage, string diskUsage)
        {
            this->_processId = id;
            this->_processName = name;
            this->_cpuTime = cpuTime;
            this->_memUsage = memUsage;
            this->_diskUsage = diskUsage;
        }
        string getProcessId() { return this->_processId; }
        string getProcessName() { return this->_processName; }
        string getCpuTime() { return this->_cpuTime; }
        string getMemUsage() { return this->_memUsage; }
        string getDiskUsage() { return this->_diskUsage; }
        string toString()
        {
            return this->_processId + "," + this->_processName + "," + this->_cpuTime + "," + this->_memUsage + "," + this->_diskUsage;
        }
    };
    class CpuTable
    {
    private:
        int _utime;
        int _stime;
        int _cutime;
        int _cstime;
        int _startTime;
        int _niceTime;
        int _upTime;
        int _cpuCount;
        int _getUpTime();

    public:
        CpuTable(int utime, int stime, int cutime, int cstime, int niceTime, int startTime)
        {
            this->_utime = utime;
            this->_stime = stime;
            this->_cutime = cutime;
            this->_cstime = cstime;
            this->_niceTime = niceTime;
            this->_startTime = startTime;
            this->_upTime = _getUpTime();
            this->_cpuCount = (int)sysconf(_SC_NPROCESSORS_ONLN);
        }

        CpuTable() {}

        int getUTime() { return _utime; }
        int getSTime() { return _stime; }
        int getCuTime() { return _cutime; }
        int getCsTime() { return _cstime; }
        int getStartTime() { return _startTime; }
        int getUpTime() { return _upTime; }
        int getCpuCount() { return _cpuCount; }
        int getNiceTime() { return _niceTime; }
    };
}

class Auth
{
    static int isValidUser(const string userName)
    {
        struct passwd *userInfo = getpwnam(userName.c_str());
        return (userInfo == nullptr) ? FAILED : SUCCESS;
    }
};

class OS
{
public:
    static int CurrentDay;
    static int CurrentMonth;
    static int CurrentYear;

    static int readRegularFiles(vector<string> &files);

    static int isDirExist(const string dirName);

    static int createDir(const string dirName);

    static int deleteFile(const string fileName);

    static int createLogFile(int cDay, int cMonth, int cYear, string &filePath, const string appName);

    static int handleLocalLogFile(int day, int month, int year, string &filePath, const string appName);

    static string isEmpty(string filename);

    static int compressFile(const string logFile);

    static string getCurretDayFileByName(string appName);
};

class AgentUtils
{

public:
    static string trim(string line);

    static void updateLogWrittenTime(const string appName, const string time);

    static int getHostName(string &host);

    static int convertTimeFormat(const std::string &inputTime, std::string &formatTime);

    static string getCurrentTime();

    static void writeLog(string log);

    static void writeLog(string log, int logLevel);
};

#endif