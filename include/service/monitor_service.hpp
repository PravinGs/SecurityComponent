#ifndef LINMONITOR_HPP
#define LINMONITOR_HPP
#pragma once

#include "common.hpp"

#define MAX_NICE_VALUE 20
#define CLK_TCK 100
#define UITME 13
#define STIME 14
#define CUTIME 15
#define CSTIME 16
#define NICETIME 18
#define START_TIME 21

typedef struct process_data process_data;
typedef struct sys_properties sys_properties;

/**
 * @brief System Properties Structure
 *
 * The `sys_properties` struct represents various system properties, including the amount of RAM (memory), disk space,
 * and CPU performance. These properties provide information about the system's hardware and resources.
 */
struct sys_properties {
    /**
     * @brief RAM (Memory) Capacity
     *
     * A double value representing the amount of RAM (memory) capacity available in the system.
     */
    double ram;

    /**
     * @brief Disk Space Capacity
     *
     * A double value representing the amount of disk space capacity available in the system.
     */
    double disk;

    /**
     * @brief CPU Performance
     *
     * A double value representing the CPU performance or processing power of the system.
     */
    double cpu;
};


/**
 * @brief Process Data Structure
 *
 * The `process_data` struct represents data related to a process, including its process ID, name, CPU time consumed,
 * memory usage, and disk usage. This structure is used to organize and store information about individual processes.
 */
struct process_data {
    /**
     * @brief Process ID
     *
     * A string representing the unique identifier of the process.
     */
    string processId;

    /**
     * @brief Process Name
     *
     * A string representing the name or identifier of the process.
     */
    string processName;

    /**
     * @brief CPU Time
     *
     * A string representing the amount of CPU time consumed by the process.
     */
    string cpuTime;

    /**
     * @brief Memory Usage
     *
     * A string representing the amount of memory (RAM) used by the process.
     */
    string memUsage;

    /**
     * @brief Disk Usage
     *
     * A string representing the amount of disk space used by the process.
     */
    string diskUsage;

    /**
     * @brief Constructor
     *
     * Initializes a `process_data` object with the provided values for process ID, name, CPU time, memory usage,
     * and disk usage.
     *
     * @param[in] id The process ID.
     * @param[in] name The process name.
     * @param[in] cTime The CPU time consumed by the process.
     * @param[in] mUsage The memory usage of the process.
     * @param[in] dUsage The disk usage of the process.
     */
    process_data(string id, string name, string cTime, string mUsage, string dUsage) 
        : processId(id), processName(name), cpuTime(cTime), memUsage(mUsage), diskUsage(dUsage)
    {}
};


/**
 * @brief CPU Table
 *
 * The `CpuTable` class contains information and values used to calculate the CPU time consumed by a process. It may
 * include data such as CPU usage percentages, process IDs, and other relevant information related to CPU monitoring
 * and resource consumption analysis.
 */
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
        : _utime(utime), _stime(stime), _cutime(cutime), _cstime(cstime), _startTime(startTime)
    {
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

/**
 * @brief Interface for Monitor Data
 *
 * The `IMonitor` class defines an interface for retrieving monitor data, which typically includes information about
 * running processes and system status. Classes that implement this interface are expected to provide concrete
 * implementations for the functions declared within.
 */
class IMonitor
{
public:

    /**
     * @brief Get and Parse Monitor Data
     *
     * The `getData` pure virtual function is used to retrieve monitor data, parse it into a JSON format, and process it.
     * This function is expected to be implemented by derived classes to provide specific functionality for retrieving
     * and processing monitor data.
     *
     * @param[in] columns A vector of column names or identifiers to specify the format and structure of the monitor data.
     * @return An integer result code:
     *         - SUCCESS: The monitor data was successfully retrieved and parsed.
     *         - FAILED: The operation encountered errors and failed to retrieve or parse the monitor data.
     */
    virtual int getData() = 0;

    /**
     * @brief Get System Properties
     *
     * The `getSystemProperties` pure virtual function is used to retrieve system properties, such as information about
     * the operating system, hardware, and other relevant system details. Derived classes implementing this function
     * are responsible for providing specific functionality to gather and return system properties.
     *
     * @return A `sys_properties` structure or object containing system properties.
     */
    virtual sys_properties getSystemProperties() = 0;

    /**
     * @brief Get Available System Properties
     *
     * The `getAvailedSystemProperties` pure virtual function is used to retrieve available system properties, which
     * represent information about the operating system, hardware, and other relevant system details that are currently
     * accessible. Derived classes implementing this function should provide specific functionality to gather and return
     * these available system properties.
     *
     * @return A `sys_properties` structure or object containing available system properties.
     */
    virtual sys_properties getAvailedSystemProperties() = 0;

    /**
     * @brief Virtual Destructor
     *
     * The virtual destructor for the `IMonitor` interface class. It ensures proper cleanup when objects of derived classes
     * are destroyed.
     */
    virtual ~IMonitor() {}
};

/**
 * @brief Monitor Service
 *
 * The `MonitorService` class is an implementation of the `IMonitor` interface, providing functionality for monitoring
 * and collecting system-related data. It gathers information about processes, system properties, and other relevant
 * data to offer insights into system performance and resource utilization.
 */
class MonitorService : public IMonitor
{
private:
    vector<std::future<void>> _asyncTasks;    
private:
    /**
     * @brief Save Process Data with Custom JSON Keys
     *
     * The `_saveLog` private method is used to store collected process data in a structured format. It takes a vector of
     * `process_data` objects representing process information and a vector of custom JSON attribute keys to specify
     * how the data should be stored in the output structure.
     *
     * @param[in] logs A vector of `process_data` objects containing process information to be stored.
     * @param[in] columns A vector of custom JSON attribute keys to specify the structure of the log data.
     * @return An integer result code:
     *         - SUCCESS: The process data was successfully stored.
     *         - FAILED: The operation encountered errors and failed to store the process data.
     */
    int _saveLog(const vector<process_data>& logs);

    /**
     * @brief Get Process Name by Process ID
     *
     * The `_getProcessNameById` private method is used to retrieve the name of a process identified by its unique
     * process ID (PID).
     *
     * @param[in] processId The unique Process ID (PID) for which the process name is to be retrieved.
     * @return A string representing the name of the process associated with the provided Process ID.
     *         If no process is found with the given ID, an empty string is returned.
     */
    string _getProcesNameById(const unsigned int& processId);

    /**
     * @brief Get All Process IDs
     *
     * The `_getProcessIds` private method is used to retrieve a vector of all active process IDs (PIDs) found within
     * the /proc directory. This function scans the directory to identify and collect the PIDs of running processes.
     *
     * @return A vector of integers representing the unique Process IDs (PIDs) of active processes.
     */
    vector<int> _getProcessIds();

    /**
     * @brief Read CPU Table for Process by ID
     *
     * The `_readProcessingTimeById` method is used to read the CPU table information for a specific process identified by its
     * unique Process ID (PID). This function retrieves the CPU time-related data for the specified process, which can be used
     * to calculate the CPU time taken by that process.
     *
     * @param[in] processId The unique Process ID (PID) of the process for which CPU time data is to be retrieved.
     * @return A `CpuTable` object containing CPU time-related information for the specified process.
     *         If the process ID is not found or there's an error reading the data, an empty `CpuTable` is returned.
     */
    CpuTable _readProcessingTimeById(const unsigned int& processId);

    /**
     * @brief Calculate CPU Time
     *
     * The `_calculateCpuTime` private method is used to calculate the actual CPU time taken based on the data in a `CpuTable`
     * object. This function processes the CPU time-related information in the provided `CpuTable` to determine the total
     * CPU time consumed.
     *
     * @param[in] table A `CpuTable` object containing CPU time-related information.
     * @return A double value representing the total CPU time taken, calculated based on the data in the `CpuTable`.
     *         If there is no valid CPU time data or an error occurs during calculation, 0.0 is returned.
     */
    double _calculateCpuTime(CpuTable& table);

    /**
     * @brief Get Memory Usage for Process by ID
     *
     * The `_getMemoryUsage` private method is used to retrieve the memory(RAM) usage of a specific process identified by its unique
     * Process ID (PID). This function retrieves the memory(RAM) usage data for the specified process.
     *
     * @param[in] processId The unique Process ID (PID) of the process for which memory(RAM) usage is to be retrieved.
     * @return A double value representing the memory(RAM) usage of the specified process in a suitable unit (e.g., megabytes).
     *         If the process ID is not found or there's an error retrieving the data, 0.0 is returned.
     */
    double _getMemoryUsage(const unsigned int& processId);

    /**
     * @brief Get Disk Usage for Process by ID
     *
     * The `_getDiskUsage` private method is used to retrieve the disk usage of a specific process identified by its unique
     * Process ID (PID). This function retrieves the disk usage data for the specified process.
     *
     * @param[in] processId The unique Process ID (PID) of the process for which disk usage is to be retrieved.
     * @return A double value representing the disk usage of the specified process in a suitable unit (e.g., megabytes).
     *         If the process ID is not found or there's an error retrieving the data, 0.0 is returned.
     */
    double _getDiskUsage(const unsigned int& processId);

    /**
     * @brief Get Write Path for Storing Process Data
     *
     * The `_getWritePath` private method is used to retrieve the file path where process data should be stored. It provides
     * the path to a location where collected process data can be saved.
     *
     * @return A string representing the file path for storing process data.
     */
    string _getWritePath();

public:
    /**
     * @brief Default Constructor for MonitorService
     *
     * The default constructor for the `MonitorService` class creates an instance of the class with default settings.
     * It initializes the object to its default state.
     */
    MonitorService() {}

    /**
     * @brief Create Process Data by Process ID
     *
     * The `createProcessData` function is used to construct a `process_data` object representing process information based
     * on the provided Process ID (PID). This function creates a `process_data` object with relevant data fields initialized.
     *
     * @param[in] processId The unique Process ID (PID) for which process data is to be created.
     * @return A `process_data` object initialized with information based on the provided Process ID.
     */
    process_data createProcessData(int processId);

    /**
     * @brief Get and Parse Monitor Data
     *
     * The `getData` function overrides the pure virtual function from the `IMonitor` class. It is used to retrieve monitor
     * data, parse it into a JSON format, and process it. Derived classes must implement this function to provide specific
     * functionality for retrieving and processing monitor data.
     *
     * @param[in] columns A vector of column names or identifiers to specify the format and structure of the monitor data.
     * @return An integer result code:
     *         - SUCCESS: The monitor data was successfully retrieved and parsed.
     *         - FAILED: The operation encountered errors and failed to retrieve or parse the monitor data.
     */
    int getData();

    /**
     * @brief Get System Properties
     *
     * The `getSystemProperties` function overrides the pure virtual function from the `IMonitor` class. It is used to
     * retrieve system properties such as RAM, disk, and CPU information. Derived classes must implement this function
     * to provide specific functionality for retrieving system properties.
     *
     * @return A `sys_properties` structure containing system-related properties.
     */
    sys_properties getSystemProperties();

    /**
     * @brief Get Available System Properties
     *
     * The `getAvailedSystemProperties` function overrides the pure virtual function from the `IMonitor` class. It is used
     * to retrieve available system properties such as RAM, disk, and CPU information. Derived classes must implement this
     * function to provide specific functionality for retrieving available system properties.
     *
     * @return A `sys_properties` structure containing available system-related properties.
     */
    sys_properties getAvailedSystemProperties();

    /**
     * @brief Destructor for MonitorService
     *
     * The destructor for the `MonitorService` class performs necessary cleanup when an instance of the class is
     * destroyed. It releases any allocated resources and performs cleanup operations.
     */
    ~MonitorService();
};

#endif