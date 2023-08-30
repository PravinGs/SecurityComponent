#ifndef AGENT_UTILS_HPP 
#define AGENT_UTILS_HPP
#define PCRE2_CODE_UNIT_WIDTH 8
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
#include <boost/asio.hpp>
#include <zlib.h>
#include <regex>
#include <pugixml.hpp>
#include <pcre2.h>
#include <future>
#include "service/croncpp.h"

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::fstream;
using std::map;
using std::string;
using std::vector;

#define DEBUG         0
#define INFO          1
#define WARNING       2
#define ERROR         3
#define CRITICAL      4
#define FATAL         5
#define ALARM         6

#define SCHEDULAR_WAIT 11

#define SUCCESS       8
#define FAILED        ERROR

const string FILE_ERROR = "File not found or permission denied: ";
const string CLEAN_FILE = "File emptied: ";
const string INVALID_FILE = "Invalid files: ";
const string FCREATION_FAILED = "File creation failed: Unable to create file ";
const string FREAD_FAILED = "Failed to read from file: Unable to read data from ";
const string FWRITE_FAILED = "Failed to write to file: Unable to write data to ";
const string FWRITE_SUCCESS = "Successfully wrote to file: Wrote data to ";
const string FDELETE_FAILED = "Failed to delete a file: Unable to delete  ";
const string FDELETE_SUCCESS = "Successfully deleted file: Deleted ";
const string INVALID_PATH = "Path not found: ";
const string NPATH = "Successfully created: ";

#define OS_SIZE_1024 1024
#define PATH_MAX 4096
#define MAX_PID 32768
#define MAX_RK_SYS 512
#define PROC_ 0
#define PID  1
#define TASK 2

#define POST_SUCCESS 200L
#define SERVER_ERROR 10
#define BASE_LOG_DIR "/etc/scl/log/"
#define BASE_LOG_ARCHIVE "archives/"
#define BASE_CONFIG_TMP "tmp/"
#define BASE_CONFIG_DIR "/etc/scl/config/"


const vector<string> MONTHS = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const int BUFFER_SIZE = 1024;
const int UDP_PORT = 8080;

typedef struct SYS_PROPERTIES SYS_PROPERTIES;
typedef struct AConfig AConfig;
typedef struct LOG_EVENT LOG_EVENT;
typedef struct Timer Timer;

struct Timer
{
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::duration<float> duration;
    Timer()
    {
        start = std::chrono::steady_clock::now();
    }
    ~Timer()
    {
        end = std::chrono::steady_clock::now();
        duration = end - start;
        float ms = duration.count() * 1000.0f;
        cout << "Timer took " << ms << "ms\n";
    }

};
struct AConfig
{
    int id;
    int level;
    int if_sid;
    int if_matched_id;
    int same_source_ip;
    int frequency;
    int timeframe;
    int same_id;
    int noalert;
    int different_url;
    int max_log_size;
    
    string group;
    string decoded_as;
    string description;
    string pcre2;
    string info;
    string type;
    string status_pcre2;
    string extra_data_pcre2;
    string options;
    string user_pcre2;
    string if_group;
    string if_matched_group;
    string id_pcre2;
    string action;
    string categories;
    string check_if_ignored;
    string ignore;
    string regex;
    string script;
    string program_name_pcre2;
    string weekday;
    string time;
    string url_pcre2;
    string if_fts;
    string hostname_pcre2;
    string match;
    string compiled_rule;

    AConfig() : id(0), level(0), if_sid(0), if_matched_id(0), same_source_ip(0), frequency(0), 
                timeframe(0), same_id(0), noalert(0), different_url(0), max_log_size(0) {}
};

struct LOG_EVENT
{
    size_t size;
    string log;
    string format; /*syslog, auth, dpkg, netstat, port*/
    string timestamp;
    string program;
    string user;
    string src_ip;
    string dest_ip;
    string proto;
    int is_matched;
    string group;
    int rule_id;

    LOG_EVENT() : size(0L), is_matched(0), rule_id(0) {}
};

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

    static int getRegularFiles(const string directory, vector<string> &files);
};

class AgentUtils
{

public:

    static string trim(string line);

    static bool isValidTimeString(const std::string& timeString);

    static void updateLogWrittenTime(const string appName, const string time);

    static int getHostName(string &host);

    static int convertTimeFormat(const std::string &inputTime, std::string &formatTime);

    static string getCurrentTime();

    static void writeLog(string log);

    static void writeLog(string log, int logLevel);

    static std::time_t convertStrToTime(const string &datetime);

};

#endif