#include "service/monitor_service.hpp"

static std::mutex p_mutex;

const string PROC = "/proc/";
const string CPUDATA = "/stat";
const string MEMORYDATA = "/statm";
const string BOOTTIME = "uptime";
const string COMM = "/comm";
const string IO = "/io";

int cpu_table::get_up_time()
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
    double start_time = table.getStartTime() / CLK_TCK;
    double elapsed_time = table.getUpTime() - start_time;
    double cpu_run_time = ((utime + stime) * 100) / elapsed_time;
    cpu_run_time = cpu_run_time * (1 + (table.getNiceTime() / MAX_NICE_VALUE));
    return cpu_run_time;
}

vector<int> monitor_service::get_processes_id() // I don't see it could be optimized
{
    int pid = -1;
    vector<int> process_ids;
    DIR *dir = opendir(PROC.c_str());
    dirent *entry = NULL;

    if (dir == NULL)
    {
        agent_utils::write_log("monitor_service: get_processes_id: " + INVALID_PATH + PROC, FAILED);
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
    string process_name = "";
    string path = PROC + std::to_string(process_id) + COMM;
    std::ifstream file(path);

    if (file.is_open())
    {
        std::getline(file, process_name);
        file.close();
    }
    return process_name;
}

double monitor_service::get_memory_usage(const unsigned int &process_id)
{
    unsigned long size, resident, shared, text, lib, data, dt;
    double memory_percentage = -1.0;
    string line;
    string path = PROC + std::to_string(process_id) + MEMORYDATA;

    long page_size = sysconf(_SC_PAGESIZE);
    long total_memory = sysconf(_SC_PHYS_PAGES) * page_size;

    std::ifstream file(path);
    if (!file)
    {
        agent_utils::write_log("monitor_service: get_memory_usage: " + FILE_ERROR + path, FAILED);
        return memory_percentage;
    }
    std::getline(file, line);
    std::istringstream iss(line);

    if (!(iss >> size >> resident >> shared >> text >> lib >> data >> dt))
    {
        agent_utils::write_log("monitor_service: get_memory_usage: failed to parse memory statistics.", FAILED);
        return memory_percentage;
    }

    memory_percentage = 100.0 * resident * page_size / total_memory;
    file.close();
    return memory_percentage;
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
        agent_utils::write_log("monitor_service: read_processing_time_id: " + FILE_ERROR + path, FAILED);
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
    double disk_usage = -1.0;
    int i = 0;
    string line;
    string path = PROC + std::to_string(process_id) + IO;
    std::fstream file(path, std::ios::in);

    if (!file.is_open())
    {
        agent_utils::write_log("monitor_service: get_disk_usage: process does not exist with this id : " + std::to_string(process_id), FAILED);
        return disk_usage;
    }
    disk_usage = 0.0;

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
                    disk_usage += d;
                }
                catch (std::exception &e)
                {
                }
            }
        }
        i++;
    }
    file.close();
    return disk_usage;
}

sys_properties monitor_service::get_system_properties()
{
    agent_utils::write_log("monitor_service: get_system_properties: request for collecting system properties started...", INFO);
    struct sys_properties properties;
    struct statvfs buffer;
    if (statvfs("/", &buffer) == 0)
    {
        unsigned long long totalSpace = buffer.f_blocks * buffer.f_frsize;
        properties.disk = static_cast<double>(totalSpace) / (1024 * 1024 * 1024);
    }
    std::ifstream meminfo(PROC + "meminfo");
    std::string line;
    unsigned long long total_memory = 0;
    unsigned long long freeMemory = 0;

    while (std::getline(meminfo, line))
    {
        if (line.find("MemTotal:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemTotal: %llu", &total_memory);
        }
        else if (line.find("MemFree:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemFree: %llu", &freeMemory);
        }
    }
    properties.ram = static_cast<double>(total_memory) / (1024 * 1024);
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
    agent_utils::write_log("monitor_service: get_availed_system_properties: request for collecting availed system properties started..", INFO);
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
    unsigned long long total_memory = 0;
    unsigned long long freeMemory = 0;

    while (std::getline(meminfo, line))
    {
        if (line.find("MemTotal:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemTotal: %llu", &total_memory);
        }
        else if (line.find("MemFree:") != std::string::npos)
        {
            sscanf(line.c_str(), "MemFree: %llu", &freeMemory);
        }
    }
    unsigned long long usedMemory = total_memory - freeMemory;
    properties.ram = static_cast<double>(usedMemory) / total_memory * 100;
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
    string process_name = get_proces_name_id(process_id);
    double cpuTime = calculate_cpu_time(table);
    double memUsage = get_memory_usage(process_id);
    double disk_usage = get_disk_usage(process_id);
    process_data processData{
        std::to_string(process_id), process_name,
        std::to_string(cpuTime), std::to_string(memUsage),
        std::to_string(disk_usage)};
    return processData;
}

int monitor_service::get_monitor_data(const process_entity &entity)
{
    agent_utils::write_log("monitor_service: get_monitor_data: request for collecting process details", DEBUG);
    vector<process_data> parent;
    vector<int> process_ids = get_processes_id();
    for (int i = 0; i < (int)process_ids.size(); i++)
    {
        int p_id = process_ids[i];
        auto asyncTask = [&, p_id]()
        {
            process_data local_data = create_process_data(p_id);
            std::lock_guard<std::mutex> lock(p_mutex);
            parent.push_back(local_data);
        };
        async_tasks.push_back(std::async(std::launch::async, asyncTask));
    }

    for (auto &task : async_tasks)
    {
        task.wait();
    }
    agent_utils::write_log("monitor_service: get_monitor_data: process information collected", DEBUG);
    sys_properties props = get_system_properties();
    sys_properties a_props = get_availed_system_properties();
    return db.save(parent, props, a_props);
}

monitor_service::~monitor_service() {}
