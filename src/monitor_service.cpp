#include "service/monitor_service.hpp"

static std::mutex p_mutex;

const string PROC       = "/proc/";
const string CPUDATA    = "/stat";
const string MEMORYDATA = "/statm";
const string BOOTTIME   = "uptime";
const string COMM       = "/comm";
const string IO         = "/io";

int cpu_table::_getUpTime()
{
    string path = PROC + BOOTTIME;
    fstream file;
    file.open(path, std::ios::in);
    string line, word;
    if (file.is_open())
    {
        std::getline(file, line);
        std::stringstream ss(line);
        std::getline(ss, word, ' ');
        file.close();
    }
    return stoi(word);
}

double monitor_service::calculate_cpu_time(cpu_table &table)
{
    double utime = table.getUTime() / CLK_TCK;
    double stime = table.getSTime() / CLK_TCK;
    double startTime = table.getStartTime() / CLK_TCK;
    double elapsedTime = table.getUpTime() - startTime;
    double cpuRunTime = ((utime + stime) * 100) / elapsedTime;
    cpuRunTime = cpuRunTime * (1 + (table.getNiceTime() / MAX_NICE_VALUE));
    return cpuRunTime;
}

vector<int> monitor_service::get_processes_id() // I don't see it could be optimized
{
    int pid = -1;
    vector<int> process_ids;
    DIR *dir = opendir(PROC.c_str());
    dirent *entry = NULL;

    if (dir == NULL)
    {
        agent_utils::write_log(INVALID_PATH + PROC, FAILED);
        return process_ids;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            try
            {
                pid = std::stoi(entry->d_name);
                process_ids.push_back(pid);
            }
            catch (const std::exception &e)
            {
            }
        }
    }
    return process_ids;
}

string monitor_service::get_proces_name_id(const unsigned int &process_id)
{
    string processName = "";
    string path = PROC + std::to_string(process_id) + COMM;
    std::ifstream file(path);

    if (file.is_open())
    {
        std::getline(file, processName);
        file.close();
    }
    return processName;
}

int monitor_service::save_monitor_log(const vector<process_data> &logs)
{
    string hostName;
    string path = os::get_json_write_path("process");
    agent_utils::get_hostname(hostName);
    sys_properties properties = get_system_properties();
    Json::Value props;
    props["CpuMemory"] = properties.cpu;
    props["RamMeomry"] = properties.ram;
    props["DiskMemory"] = properties.disk;
    properties = get_availed_system_properties();
    Json::Value availedProps;
    availedProps["CpuMemory"] = properties.cpu;
    availedProps["RamMeomry"] = properties.ram;
    availedProps["DiskMemory"] = properties.disk;
    fstream file(path, std::ios::app);
    Json::Value jsonData;
    Json::StreamWriterBuilder writerBuilder;
    if (!file)
    {
        agent_utils::write_log(FWRITE_FAILED + path, FAILED);
        return FAILED;
    }
    
    jsonData["DeviceTotalSpace"] = props;
    jsonData["DeviceUsedSpace"] = availedProps;
    jsonData["TimeGenerated"] = agent_utils::get_current_time();
    jsonData["Source"] = hostName;
    jsonData["OrgId"] = 12345;
    jsonData["ProcessObjects"] = Json::Value(Json::arrayValue);
    for (process_data data : logs)
    {
        Json::Value jsonLog;
        jsonLog["process_id"] = std::stoi(data.process_id);
        jsonLog["process_name"] = data.processName;
        jsonLog["cpu_usage"] = std::stod(data.cpuTime);
        jsonLog["ram_usage"] = std::stod(data.memUsage);
        jsonLog["disk_usage"] = std::stod(data.diskUsage);
        jsonData["ProcessObjects"].append(jsonLog);
    }

    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(jsonData, &file);

    file.close();
    agent_utils::write_log(FWRITE_SUCCESS + path, SUCCESS);
    return SUCCESS;
}

double monitor_service::get_memory_usage(const unsigned int &process_id)
{
    unsigned long size, resident, shared, text, lib, data, dt;
    double memoryPercentage = -1.0;
    string line;
    string path = PROC + std::to_string(process_id) + MEMORYDATA;

    long pageSize = sysconf(_SC_PAGESIZE);
    long totalMemory = sysconf(_SC_PHYS_PAGES) * pageSize;

    std::ifstream file(path);
    if (!file)
    {
        agent_utils::write_log(FILE_ERROR + path, FAILED);
        return memoryPercentage;
    }
    std::getline(file, line);
    std::istringstream iss(line);

    if (!(iss >> size >> resident >> shared >> text >> lib >> data >> dt))
    {
        agent_utils::write_log("Failed to parse memory statistics.", FAILED);
        return memoryPercentage;
    }

    memoryPercentage = 100.0 * resident * pageSize / totalMemory;
    file.close();
    return memoryPercentage;
}

cpu_table monitor_service::read_processing_time_id(const unsigned int &process_id)
{
    string path = PROC + std::to_string(process_id) + CPUDATA;
    vector<string> stats;
    string line, word;
    fstream file;

    file.open(path, std::ios::in);
    if (!(file.is_open()))
    {
        cpu_table emptyTable;
        agent_utils::write_log(FILE_ERROR + path, FAILED);
        return emptyTable;
    }
    std::getline(file, line);
    std::stringstream ss(line);
    while (std::getline(ss, word, ' '))
    {
        stats.push_back(word);
    }
    file.close();
    cpu_table table{
        stoi(stats.at(UITME)), stoi(stats.at(STIME)),
        stoi(stats.at(CUTIME)), stoi(stats.at(CSTIME)),
        stoi(stats.at(NICETIME)), stoi(stats.at(START_TIME))};
    return table;
}

double monitor_service::get_disk_usage(const unsigned int &process_id)
{
    double diskUsage = -1.0;
    int i = 0;
    string line;
    string path = PROC + std::to_string(process_id) + IO;
    std::fstream file(path, std::ios::in);

    if (!file.is_open())
    {
        agent_utils::write_log("Process does not exist with this id : " + std::to_string(process_id), FAILED);
        return diskUsage;
    }
    diskUsage = 0.0;

    while (std::getline(file, line))
    {
        string token;
        std::stringstream ss(line);
        if (i == 4 || i == 5)
        {
            while (std::getline(ss, token, ':'))
            {
                try
                {
                    double d = std::stod(token);
                    diskUsage += d;
                }
                catch (std::exception &e)
                {
                }
            }
        }
        i++;
    }
    file.close();
    return diskUsage;
}

sys_properties monitor_service::get_system_properties()
{
    agent_utils::write_log("Request for collecting system properties started...", INFO);
    struct sys_properties properties;
    struct statvfs buffer;
    if (statvfs("/", &buffer) == 0)
    {
        unsigned long long totalSpace = buffer.f_blocks * buffer.f_frsize;
        properties.disk = static_cast<double>(totalSpace) / (1024 * 1024 * 1024);
    }
    std::ifstream meminfo(PROC + "meminfo");
    std::string line;
    unsigned long long totalMemory = 0;
    unsigned long long freeMemory = 0;

    while (std::getline(meminfo, line))
    {
        if (line.find("MemTotal:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemTotal: %llu", &totalMemory);
        }
        else if (line.find("MemFree:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemFree: %llu", &freeMemory);
        }
    }
    properties.ram = static_cast<double>(totalMemory) / (1024 * 1024);
    meminfo.close();
    std::ifstream statFile("/proc/stat");
    unsigned long long userTime = 0;
    unsigned long long niceTime = 0;
    unsigned long long systemTime = 0;
    unsigned long long idleTime = 0;

    while (std::getline(statFile, line))
    {
        if (line.find("cpu ") != std::string::npos)
        {
            sscanf(line.c_str(), "cpu %llu %llu %llu %llu", &userTime, &niceTime, &systemTime, &idleTime);
            break;
        }
    }
    unsigned long long totalTime = userTime + niceTime + systemTime + idleTime;
    properties.cpu = static_cast<double>(totalTime);
    statFile.close();
    return properties;
}

sys_properties monitor_service::get_availed_system_properties()
{
    agent_utils::write_log("Request for collecting availed system properties started..", INFO);
    struct sys_properties properties;
    struct statvfs buffer;
    if (statvfs("/", &buffer) == 0)
    {
        unsigned long long totalSpace = buffer.f_blocks * buffer.f_frsize;
        unsigned long long availableSpace = buffer.f_bavail * buffer.f_frsize;
        unsigned long long usedSpace = totalSpace - availableSpace;
        properties.disk = static_cast<double>(usedSpace) / totalSpace * 100;
    }
    std::ifstream meminfo(PROC + "meminfo");
    std::string line;
    unsigned long long totalMemory = 0;
    unsigned long long freeMemory = 0;

    while (std::getline(meminfo, line))
    {
        if (line.find("MemTotal:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemTotal: %llu", &totalMemory);
        }
        else if (line.find("MemFree:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemFree: %llu", &freeMemory);
        }
    }
    unsigned long long usedMemory = totalMemory - freeMemory;
    properties.ram = static_cast<double>(usedMemory) / totalMemory * 100;
    meminfo.close();
    std::ifstream statFile("/proc/stat");
    unsigned long long userTime = 0;
    unsigned long long niceTime = 0;
    unsigned long long systemTime = 0;
    unsigned long long idleTime = 0;

    while (std::getline(statFile, line))
    {
        if (line.find("cpu ") != std::string::npos)
        {
            sscanf(line.c_str(), "cpu %llu %llu %llu %llu", &userTime, &niceTime, &systemTime, &idleTime);
            break;
        }
    }

    unsigned long long totalTime = userTime + niceTime + systemTime + idleTime;
    properties.cpu = (static_cast<double>(totalTime) - idleTime) / totalTime * 100;
    statFile.close();
    return properties;
}

process_data monitor_service::create_process_data(int process_id)
{
    cpu_table table = read_processing_time_id(process_id);
    string processName = get_proces_name_id(process_id);
    double cpuTime = calculate_cpu_time(table);
    double memUsage = get_memory_usage(process_id);
    double diskUsage = get_disk_usage(process_id);
    process_data processData{
        std::to_string(process_id), processName,
        std::to_string(cpuTime), std::to_string(memUsage),
        std::to_string(diskUsage)};
    // data.push_back(processData);
    return processData;
}

int monitor_service::get_monitor_data()
{
    agent_utils::write_log("Request for collecting process details", DEBUG);
    vector<process_data> parent;
    vector<int> process_ids = get_processes_id();
    for (int i = 0; i < (int)process_ids.size(); i++)
    {
        int p_id = process_ids[i];
        auto asyncTask = [&, p_id]()
        {
            // vector<process_data> localData;
            process_data localData = create_process_data(p_id);
            std::lock_guard<std::mutex> lock(p_mutex);
            parent.push_back(localData);
            // parent.insert(parent.end(), localData.begin(), localData.end());
        };
        _async_tasks.push_back(std::async(std::launch::async, asyncTask));
    }

    // Wait for async tasks to complete
    for (auto &task : _async_tasks)
    {
        task.wait();
    }
    agent_utils::write_log("Process information collected", DEBUG);
    return save_monitor_log(parent);
}

monitor_service::~monitor_service() {}
