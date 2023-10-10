#ifndef log_service_HPP
#define log_service_HPP
#pragma once

#include "common.hpp"
#include "service/config_service.hpp"
#include "udp.hpp"

typedef struct standard_log_attrs standard_log_attrs;

static map<string, int> LogCategory {{"sys", 2}, {"network", 3}, {"ufw", 4}};

struct standard_log_attrs
{
    string timestamp;
    string user;
    string program;
    string message;
    int level;
    int category;

    standard_log_attrs(const string& log)
    {
        string t_level, t_category;
        std::stringstream ss(log);
        std::getline(ss, timestamp, '|');
        std::getline(ss, user, '|');
        std::getline(ss, program, '|');
        std::getline(ss, message, '|');
        std::getline(ss, t_level, '|');
        std::getline(ss, t_category, '|');
        level = handle_exception(t_level);
        category = LogCategory[t_category];
    }

    int handle_exception(const string& level)
    {
        int r;
        try
        {
            r = std::stoi(level);
        }
        catch(const std::exception& e)
        {
            r = 0;
        }
        return r;
    }

    ~standard_log_attrs(){}
    
};

/**
 * @brief Abstract Log Interface
 *
 * The `ILog` abstract class defines a common interface for log-related operations within the application. It declares
 * several pure virtual functions for reading and managing logs. Derived classes are expected to provide concrete
 * implementations for these functions to handle specific types of logs, such as syslog and application logs.
 */
class ILog
{
public:
    /**
     * @brief Get Syslog
     *
     * This pure virtual function is meant to be implemented by derived classes. It defines the contract for retrieving
     * syslog data based on the provided parameters.
     *
     * @param[in] app_name The name of the application for which syslog data is requested.
     * @param[in] json A JSON object to store syslog information.
     * @param[in] log_attributes A vector of log_attributes specifying the cetralized log_attributes to retrieve. (TimeGenerated,UserLoginId,LogLevel,Priority,Message)
     * @param[in] read_path The read_path to the syslog file.
     * @param[in,out] last_read_time The timestamp indicating the last read time of the syslog file.
     * @param[in] log_levels A vector of log log_levels to filter syslog entries.(customizable log_levels to read)
     * @param[in] remote A character indicating whether a remote connection is established ('Y' for yes, 'N' for no).
     * @return An integer result code:
     *         - SUCCESS: The syslog retrieval operation was successful.
     *         - FAILED: The syslog retrieval operation encountered errors.
     */
    virtual int get_syslog(
        const string& app_name,
        Json::Value &json,
        const vector<string>& log_attributes,
        const string& read_path,
        string &last_read_time,
        const vector<string>& log_levels,
        const char& remote) = 0;

    /**
     * @brief Get Application Log
     *
     * This pure virtual function is meant to be implemented by derived classes. It defines the contract for retrieving
     * application log data based on the provided parameters.
     *
     * @param[in] json A JSON object to store application log information.
     * @param[in] log_attributes A vector of log_attributes specifying the application log entries to retrieve. (log log_attributes)
     * @param[in] read_path The directory where application log files are located.
     * @param[in] write_path The read_path to write processed application log data.
     * @param[in,out] last_read_time The timestamp indicating the last read time of the log files.
     * @param[in] log_levels A vector of log log_levels to filter application log entries.
     * @param[in] delimiter A character delimiter for parsing log entries.
     * @return An integer result code:
     *         - SUCCESS: The application log retrieval operation was successful.
     *         - FAILED: The application log retrieval operation encountered errors.
     */
    virtual int get_applog(
        Json::Value &json,
        const vector<string>& log_attributes,
        const string& read_path,
        const string& write_path,
        string &last_read_time,
        const vector<string>& log_levels,
        const char& delimeter) = 0;
    
    /**
     * @brief Virtual Destructor
     *
     * The virtual destructor ensures proper cleanup when objects of derived classes are destroyed.
     */
    virtual ~ILog() {}
};

/**
 * @brief Log Service Implementation
 *
 * The `log_service` class is a concrete implementation of the `ILog` interface. It provides functionality for
 * retrieving and managing log data, including syslog and application logs. This class is responsible for
 * implementing the log-related operations defined in the `ILog` interface.
 */
class log_service : public ILog
{
private:
    Config _config_service; /**< A private instance of IniConfig for configuration management. */
    map<string, int> _logLevel{{"none", 0}, {"trace", 1}, {"debug", 2}, {"warning", 3}, {"error", 4}, {"critical", 5}}; /**< A private constant map<string, int> for system log name. */
    
private:
    /**
     * @brief Save Log Data as JSON
     *
     * The `_save_json` function is a private method used to create a JSON file and store processed log data in it. It takes the provided `json`
     * object, log data from the `logs` vector, and column information from the `columns` vector to create the JSON file at
     * the specified `read_path`. The `delimeter` parameter is used as a separator in the log data when constructing the JSON.
     *
     * @param[in] json A JSON object to store the log data.
     * @param[in] read_path The read_path where the JSON file will be created and saved.
     * @param[in] logs A vector of log entries to be included in the JSON.
     * @param[in] columns A vector of column information for the JSON structure.
     * @param[in] delimeter The character delimiter used to separate log data.
     * @return An integer result code:
     *         - SUCCESS: The log data was successfully saved as a JSON file.
     *         - FAILED: The operation encountered errors and failed to save the JSON file.
     */
    int save_json(Json::Value &json, const string& read_path, const vector<string>& logs, const vector<string>& columns, const char& delimeter);

    /**
     * @brief Read Syslog File
     *
     * The `read_syslog_file` function is a private method used to read a syslog file located at the specified `read_path`. It splits
     * the raw log lines into formatted log entries, which are stored in the `logs` parameter as a vector. The `delimeter` parameter is used as a separator when parsing log lines. The `last_read_time`
     * parameter is used to specify the last read time of the log file, and the `flag` parameter is set to `true` if the end
     * of the file is reached. Log entries are filtered based on the specified `log_levels`. The `next_log_reading_time` parameter is
     * updated to indicate the next time to read the log file.
     *
     * @param[in] read_path The read_path to the syslog file.
     * @param[in, out] logs A vector of formatted log entries.
     * @param[in] delimeter The character delimiter used to separate log data.
     * @param[in] last_read_time The last read time of the log file.
     * @param[in, out] flag A boolean flag indicating log file read completed.
     * @param[in] log_levels A vector of log log_levels to filter log entries.
     * @param[in,out] next_log_reading_time The timestamp indicating the next time to read the log file.
     * @return An integer result code:
     *         - SUCCESS: The syslog reading operation was successful.
     *         - FAILED: The syslog reading operation encountered errors.
     */
    int read_syslog_file(const string& read_path, vector<string> &logs, const char& delimeter, const string &last_read_time, bool &flag, const vector<string>& log_levels, string &next_log_reading_time);

    /**
     * @brief Read Application Log File
     *
     * The `read_applog_file` function is a private method used to read an application log file located at the specified `read_path`.
     * It splits the raw log lines into formatted log entries, which are stored in the `logs` parameter as a vector. The
     * `delimeter` parameter is used as a separator when parsing log lines. The `last_read_time` parameter is used to specify
     * the last read time of the log file, and the `flag` parameter is set to `true` if the end of the file is reached.
     * Log entries are filtered based on the specified `log_levels`. The `next_log_reading_time` parameter is updated to indicate the
     * next time to read the log file.
     *
     * @param[in] read_path The read_path to the application log file.
     * @param[in, out] logs A vector of formatted log entries.
     * @param[in] delimeter The character delimiter used to separate log data.
     * @param[in] last_read_time The last read time of the log file.
     * @param[in, out] flag A boolean flag indicating if the end of the file is reached.
     * @param[in] log_levels A vector of log log_levels to filter log entries.
     * @param[in,out] next_log_reading_time The timestamp indicating the next time to read the log file.
     * @return An integer result code:
     *         - SUCCESS: The application log reading operation was successful.
     *         - FAILED: The application log reading operation encountered errors.
     */
    int read_applog_file(const string& read_path, vector<string> &logs, const char& delimeter, const string &last_read_time, bool &flag, const vector<string>& log_levels, string &next_log_reading_time);
    
    /**
     * @brief Get Files in Directory
     *
     * The `get_dir_files` function is a private method used to retrieve a list of regular files located in the specified `directory`.
     * It returns a vector containing `std::filesystem::directory_entry` objects representing the files found in the directory.
     *
     * @param[in] directory The read_path to the directory from which to extract files.
     * @return A vector of `std::filesystem::directory_entry` objects representing the files in the directory.
     */
    vector<std::filesystem::directory_entry> get_dir_files(const string& directory);

    /**
     * @brief Save Logs to Local Storage
     *
     * The `save_read_logs` function is a private method used to store collected logs from the provided `logs` vector to local storage.
     * The logs are associated with the specified `app_name`. This function is responsible for persisting log data
     * locally for further analysis or storage.
     *
     * @param[in] logs A vector of log entries to be saved to local storage.
     * @param[in] app_name The name of the application associated with the logs.
     * @return An integer result code:
     *         - SUCCESS: The log data was successfully saved to local storage.
     *         - FAILED: The operation encountered errors and failed to save the log data.
     */
    int save_read_logs(const vector<string>& logs, const string& app_name);

    /**
     * @brief Verify JSON read_path
     *
     * The `get_json_write_path` function is a private method used to verify the existence of a file specified by the provided `timestamp`.
     * If the file does not exist, this function creates a new file at the specified read_path. It ensures that the read_path is
     * available for storing JSON data.
     *
     * @param[in,out] timestamp The read_path to the file to be verified or created. If the file does not exist, a new file
     *                          will be created at this read_path.
     * @return An integer result code:
     *         - SUCCESS: The file read_path was verified, and the file exists or was successfully created.
     *         - FAILED: The operation encountered errors, and the file read_path could not be verified or created.
     */
    int get_json_write_path(string &timestamp);

    /**
     * @brief Categorize Log Entry
     *
     * The `categorize` function is a private method used to categorize a log entry specified by the `line` parameter into particular categories
     * based on their data and the provided `log_levels`. This function is responsible for assigning a category or label to a log entry
     * to aid in log analysis or organization.
     *
     * @param[in, out] line The log entry to be categorized.
     * @param[in] log_levels A vector of log log_levels used to determine the categorization criteria.
     */
    void categorize(string &line, const vector<string>& log_levels);
    
    /**
     * @brief Read Dpkg Log File
     *
     * The `read_dpkg_logfile` function is a private method used to read a dpkg log file located at the specified `read_path`. It parses the
     * log lines into individual log entries, which are stored in the `logs` vector. The `last_read_time` parameter is used
     * to specify the last read time of the log file. The `next_log_reading_time` parameter is updated to indicate the next time
     * to read the log file. The `flag` parameter is set to `true` if the end of the file is reached.
     *
     * @param[in] read_path The read_path to the dpkg-formatted log file.
     * @param[in, out] logs A vector of parsed log entries.
     * @param[in, out] last_read_time The last read time of the log file.
     * @param[in, out] next_log_reading_time The timestamp indicating the next time to read the log file.
     * @param[in, out] flag A boolean flag indicating if the end of the file is reached.
     * @return An integer result code:
     *         - SUCCESS: The dpkg-formatted log reading operation was successful.
     *         - FAILED: The dpkg-formatted log reading operation encountered errors.
     */
    int read_dpkg_logfile(const string& read_path, vector<string> &logs, string &last_read_time, string &next_log_reading_time, bool &flag);

    /**
     * @brief Read Remote Syslog Data
     *
     * The `read_remote_syslog` function is used to read syslog data from a remote source connected through UDP. It retrieves
     * syslog data from the `queue` and stores it in the `logs` vector. This function is responsible for reading logs
     * from a remote source when a remote connection is established.
     *
     * @param[in] queue The UDP queue for receiving syslog data from a remote source.
     * @param[in, out] logs A vector to store the received syslog data.
     * @return An integer result code:
     *         - SUCCESS: The remote syslog reading operation was successful.
     *         - FAILED: The remote syslog reading operation encountered errors.
     */
    int read_remote_syslog(UdpQueue &queue, vector<string> &logs);

public:
    /**
     * @brief Default Constructor
     *
     * The `log_service` class default constructor. It initializes a `log_service` object using the default constructor provided
     * by the compiler.
     */
    log_service() = default;

    /**
     * @brief Get Syslog Data
     *
     * The `get_syslog` function is an overload of the pure virtual function defined in the `ILog` interface. It acts as a manager
     * for collecting syslog data from various sources. This function retrieves syslog data from the specified `read_path` and processes
     * it based on the provided criteria such as `app_name`, `log_attributes`, `log_levels`, and `remote`. The `json` object is used to store the
     * collected syslog data, and the `last_read_time` parameter is utilized to keep track of the last read time.
     *
     * @param[in] app_name The name of the application for which syslog data is collected.
     * @param[in] json A JSON object to store the collected syslog data.
     * @param[in] log_attributes A vector of log file identifiers to filter syslog data.
     * @param[in] read_path The read_path to the syslog file(s) to be read.
     * @param[in,out] last_read_time The last read time of the syslog file(s).
     * @param[in] log_levels A vector of log log_levels to filter syslog entries.
     * @param[in] remote A character indicating whether syslog data is collected remotely.
     * @return An integer result code:
     *         - SUCCESS: The syslog data was successfully collected and processed.
     *         - FAILED: The syslog data collection encountered errors.
     */
    int get_syslog(const string& app_name, Json::Value &json, const vector<string>& log_attributes, const string& read_path, string &last_read_time, const vector<string>& log_levels, const char& remote); 

    /**
     * @brief Get Application Log Data
     *
     * The `get_applog` function is an implementation of a virtual function defined in the `ILog` interface. It retrieves
     * application log data from the specified `read_path` and processes it based on the provided criteria such as `log_attributes`,
     * `log_levels`, and `delimeter`. The `json` object is used to store the collected application log data, and the `last_read_time`
     * parameter is utilized to keep track of the last read time. The processed log data is written to the specified `write_path`.
     *
     * @param[in] json A JSON object to store the collected application log data.
     * @param[in] log_attributes A vector of log file identifiers to filter application log data.
     * @param[in] read_path The directory containing application log files to be read.
     * @param[in] write_path The read_path where the processed log data will be written.
     * @param[in,out] last_read_time The last read time of the application log files.
     * @param[in] log_levels A vector of log log_levels to filter application log entries.
     * @param[in] delimeter The character delimiter used to separate log data.
     * @return An integer result code:
     *         - SUCCESS: The application log data was successfully collected and processed.
     *         - FAILED: The application log data collection encountered errors.
     */
    int get_applog(Json::Value &json, const vector<string>& log_attributes, const string& read_path, const string& write_path, string &last_read_time, const vector<string>& log_levels, const char& delimeter);

     /**
     * @brief Destructor for log_service.
     *
     * The destructor performs cleanup tasks for the `log_service` class, which may include
     * releasing resources and deallocating memory.
     */
    ~log_service();
};

#endif