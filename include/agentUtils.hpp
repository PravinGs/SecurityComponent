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
const string LOG_PATH = "/etc/scl/log/agent.log";
const int BUFFER_SIZE = 1024;
const int UDP_PORT = 8080;

typedef struct AConfig AConfig;
typedef struct log_event log_event;
typedef struct Timer Timer;
typedef struct decoder decoder;



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
    vector<string> pcre2;
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

/**
 * @brief A structure representing a log event with various attributes.
 *
 * The log_event structure encapsulates information related to a log event,
 * including its size, original log entry, log format, timestamp, program,
 * user, source and destination IP addresses, protocol, match status, group,
 * and associated rule ID.
 */
struct log_event
{
    size_t size;            /**< The size of the log event. */
    string log;             /**< The original log entry string. */
    string format;          /**< The log format (e.g., syslog, auth, dpkg, netstat, port). */
    string timestamp;       /**< The timestamp associated with the log event. */
    string program;         /**< The program or source of the log event. */
    string user;            /**< The user associated with the log event. */
    string message;
    string src_ip;          /**< The source IP address in the log event. */
    string dest_ip;         /**< The destination IP address in the log event. */
    string proto;           /**< The protocol used in the log event. */
    int is_matched;         /**< A flag indicating if the log event matched a rule (0 or 1). */
    string group;           /**< The group associated with the matched rule. */
    int rule_id;            /**< The ID of the rule that matched the log event. */

    /**
     * @brief Default constructor for the log_event structure.
     *
     * Initializes an instance of the log_event structure with default values
     * for its attributes.
     */
    log_event() : size(0L), is_matched(0), rule_id(0) {}
};

struct decoder
{
    string decode;
    string parent;
    string program_name_pcre2;
    string pcre2;
    string order;
    string prematch_pcre2;
    string fts;
    string prematch_offset;
    string pcre2_offset;

    void update(const decoder& other)
    {
        if (decode.empty()) decode = other.decode;
        if (parent.empty()) parent = other.parent;
        if (program_name_pcre2.empty()) program_name_pcre2 = other.program_name_pcre2;
        if (pcre2.empty()) pcre2 = other.pcre2;
        if (order.empty()) order = other.order;
        if (prematch_pcre2.empty()) prematch_pcre2 = other.prematch_pcre2;
        if (fts.empty()) fts = other.fts;
        if (prematch_offset.empty()) prematch_offset = other.prematch_offset;
        if (pcre2_offset.empty()) pcre2_offset = other.pcre2_offset;
    }
};

/**
 * @brief A utility class for managing file and directory operations and maintaining a global timestamp.
 *
 * The OS class provides a set of methods and functionality for performing various file and
 * directory operations, such as creation, reading, updating, and deletion (CRUD), as well as
 * maintaining a global timestamp for the agent application.
 *
 * This class serves as a utility for handling operating system-related tasks in the context of
 * the agent application.
 */
class OS
{
public:
    static int CurrentDay;
    static int CurrentMonth;
    static int CurrentYear;

    /**
     * @brief Retrieve a list of regular files from a directory.
     *
     * This static method scans a specified directory and collects the paths of all regular files
     * (excluding directories and special files) it contains. The list of file paths is stored in
     * the provided vector.
     *
     * @param files A vector to store the paths of regular files found in the directory.
     *
     * @return The number of regular files found and added to the vector.
     */
    static int readRegularFiles(vector<string> &files);

    /**
     * @brief Check if a directory exists.
     *
     * This static method checks if a directory with the specified name exists.
     *
     * @param dirName The name of the directory to check.
     *
     * @return An integer indicating the result:
     *         - SUCCESS if the directory exists.
     *         - FAILED if the directory does not exist.
     */
    static int isDirExist(const string& dirName);

    /**
     * @brief Create a directory.
     *
     * This static method attempts to create a directory with the specified name.
     *
     * @param dirName The name of the directory to create.
     *
     * @return An integer indicating the result:
     *         - SUCCESS if the directory was successfully created.
     *         - FAILED if an error occurred during the creation.
     */
    static int createDir(const string dirName);

    /**
     * @brief Delete a file.
     *
     * This static method attempts to delete a file with the specified name.
     *
     * @param fileName The name of the file to delete.
     *
     * @return An integer indicating the result:
     *         - SUCCESS if the file was successfully deleted.
     *         - FAILED if an error occurred during the deletion.
     */
    static int deleteFile(const string& fileName);

    /**
     * @brief Create a log file for storing processed logs under a specified date and application name.
     *
     * This static method creates a log file for storing processed logs under a directory structure
     * organized by year, month, and day. The log file's name is composed of the provided year, month,
     * day, and application name.
     *
     * @param cDay The current day of the month.
     * @param cMonth The current month.
     * @param cYear The current year.
     * @param filePath A reference to a string to store the path of the created log file.
     * @param appName The name of the application or context associated with the log file.
     *
     * @return An integer indicating the result:
     *         - 0 if the log file was successfully created.
     *         - (-1) if an error occurred during file creation.
     */
    static int createLogFile(int cDay, int cMonth, int cYear, string& filePath, const string& appName);

    /**
     * @brief Manage log files and backup previous day's files based on the date.
     *
     * This method manages log files, including creating a new log file for the current day
     * and backing up log files from the previous day when a new day arrives. Log files are
     * organized by year, month, and day, and named based on the provided date and application
     * name.
     *
     * @param day The current day of the month.
     * @param month The current month.
     * @param year The current year.
     * @param filePath A reference to a string to store the path of the created log file.
     * @param appName The name of the application or context associated with the log files.
     *
     * @return An integer indicating the result:
     *         - 0 if log files were managed successfully.
     *         - (-1) if an error occurred during log file management.
     */
    static int handleLocalLogFile(int day, int month, int year, string& filePath, const string& appName);

    static string isEmpty(string filename);

    /**
     * @brief Compress a file using a compression algorithm.
     *
     * This static method compresses a specified file using a compression algorithm.
     *
     * @param logFile The path to the file to be compressed.
     *
     * @return An integer indicating the result:
     *         - 0 if the file was successfully compressed.
     *         - (-1) if an error occurred during the compression process.
     */
    static int compressFile(const string& logFile);

    /**
     * @brief Generate a file path for the application name based on the current date.
     *
     * This static method generates a file path for the specified application name based on
     * the current date, organizing files by year, month, and day.
     *
     * @param appName The name of the application for which the file path is generated.
     *
     * @return A string containing the file path based on the current date and application name.
     */
    static string getCurretDayFileByName(const string& appName);

    static int getRegularFiles(const string& directory, vector<string> &files);
};

/**
 * @brief Utility class for implementing functions and log mechanisms in the agent application.
 *
 * The AgentUtils class provides a set of utility methods and log mechanisms to support various
 * functionalities within the agent application. It encapsulates common tasks related to agent
 * operation and logging.
 */
class AgentUtils
{
private:
    
public:
    static fstream logfp;

    static string trim(string line);

    /**
     * @brief Validate a syslog time string.
     *
     * This static method checks whether a given string represents a valid syslog time string.
     *
     * @param timeString The string to be validated as a syslog time string.
     *
     * @return A boolean value indicating the validation result:
     *         - true if the string is a valid syslog time string.
     *         - false if the string is not a valid syslog time string.
     */
    static bool isValidTimeString(const std::string& timeString);

    /**
     * @brief Update the last written time for a specific application's log.
     *
     * This static method updates the last written time for a specified application's log.
     *
     * @param appName The name of the application whose log time is being updated.
     * @param time The time to be set as the last written time for the application's log.
     */
    static void updateLogWrittenTime(const string& appName, const string& time);

    static int getHostName(string &host);

    /**
     * @brief Convert a syslog time format into a standard time format.
     *
     * This static method takes a syslog time format string as input and converts it into
     * a standard time format string, storing the result in the 'formatTime' parameter.
     *
     * @param inputTime The syslog time format string to be converted.
     * @param formatTime A reference to a string to store the converted standard time format.
     *
     * @return An integer indicating the result:
     *         - 0 if the conversion was successful.
     *         - (-1) if an error occurred during the conversion.
     */
    static int convertTimeFormat(const std::string &inputTime, std::string &formatTime);

    static string getCurrentTime();

    static void writeLog(const string& log);

    static void writeLog(const string& log, int logLevel);

    static std::time_t convertStrToTime(const string &datetime);

    ~AgentUtils() 
    {
        if (logfp.is_open())
            logfp.close();
    }

};

#endif