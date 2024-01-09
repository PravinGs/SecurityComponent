#ifndef AGENT_UTILS_HPP
#define AGENT_UTILS_HPP
#define PCRE2_CODE_UNIT_WIDTH 8
#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <unordered_map>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <syslog.h>
#include <json/json.h>
#include <filesystem>
#include <chrono>
#include <thread>
#if RABBITMQ
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ev.h>
#endif

#include <tss2/tss2_esys.h>
#include <tss2/tss2_tctildr.h>

#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <csignal>
#include <filesystem>
#include <curl/curl.h>
#include <zlib.h>
#include <regex>
#include <pugixml.hpp>
#include <pcre2.h>
#include <future>
#include <mqtt/async_client.h>
#include "service/croncpp.h"

using std::cerr;
using std::cout;
using std::exception;
using std::fstream;
using std::map;
using std::string;
using std::vector;

#define DEBUG 0
#define INFO 1
#define WARNING 2
#define ERROR 3
#define CRITICAL 4
#define FATAL 5
#define ALARM 6
#define SCHEDULAR_WAIT 11
#define SUCCESS 8
#define FAILED ERROR
#define OS_SIZE_1024 1024
#define PATH_MAX 4096
#define MAX_PID 32768
#define MAX_RK_SYS 512
#define PROC_ 0
#define PID 1
#define TASK 2

#define POST_SUCCESS 200L
#define SERVER_ERROR 10

#define BASE_LOG_DIR "/etc/scl/log/"
#define BASE_LOG_ARCHIVE "archives/"
#define BASE_CONFIG_TMP "tmp/"
#define BASE_CONFIG_DIR "/etc/scl/config/"
#define AGENT_CONFIG_DIR "/etc/scl/config/agent/agent.config"
#define AGENT_TEMP_DIR "/etc/scl/tmp/"

const string FILE_ERROR = "file not found or permission denied: ";
const string CLEAN_FILE = "file truncated: ";
const string INVALID_FILE = "invalid files: ";
const string FCREATION_FAILED = "file creation failed: unable to create file ";
const string FREAD_FAILED = "failed to read from file: unable to read data from ";
const string FWRITE_FAILED = "failed to write to file: unable to write data to ";
const string FWRITE_SUCCESS = "successfully wrote to file: wrote data to ";
const string FDELETE_FAILED = "failed to delete a file: unable to delete  ";
const string FDELETE_SUCCESS = "successfully deleted file: deleted ";
const string INVALID_PATH = "path not found: ";
const string NEW_PATH = "successfully created: ";
const string APP = "agent";
const string LOG_PATH = "/etc/scl/log/agent.log";
const vector<string> MONTHS = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const int BUFFER_SIZE = 1024;
const int UDP_PORT = 8080;

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

class os
{
public:
    static int current_day;
    static int current_month;
    static int current_year;
    static string host_name;

    static bool is_dir_exist(const string &dirName);

    static int create_dir(const string& dirName);

    static int create_file(const string& file_path);

    static int delete_file(const string &fileName);

    static int create_log_file(int cDay, int cMonth, int cYear, string &filePath, const string &app_name);

    static int handle_local_log_file(int day, int month, int year, string &filePath, const string &app_name);

    static string get_path_or_backup_file_path(string filename);

    static int compress_file(const string &logFile);

    static string get_file_by_current_day(const string &app_name);

    static int get_regular_files(const string &directory, vector<string> &files);

    static string get_json_write_path(const string &type);

    static bool is_file_exist(const string &file);

    static string sign(const string& file, const string&sign_key);

    static bool verify_signature(const string& file, const string& sign_key, const string&signed_data);

    static string get_json_file_path();
};


class agent_utils
{
private:
public:
    static fstream logfp;

    static bool syslog_enabled;

    static string trim(string line);

    static bool debug;

    static void setup_logger();

    static void backup_log_file();

    static string to_lower_case(string &str);

    static bool is_valid_time_string(const std::string &time_string);

    static void update_log_written_time(const string &app_name, const string &time);

    static int get_hostname(string &host);

    static int convert_time_format(const std::string &input_time, std::string &format_time);

    static string get_current_time();

    static void write_log(const string &log);

    static void write_log(const string &log, int logLevel);

    static std::time_t string_to_time_t(const string &datetime);

    static void print_next_execution_time(std::tm *next_time_info);

    static void print_duration(const std::chrono::duration<double> &duration);

    ~agent_utils()
    {
        if (logfp.is_open())
            logfp.close();
    }
};

#endif