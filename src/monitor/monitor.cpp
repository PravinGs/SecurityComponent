#include "monitor/monitor.hpp"

const string PROC = "/proc/";
const string CPUDATA = "/stat";
const string MEMORYDATA = "/statm";
const string BOOTTIME = "uptime";
const string COMM = "/comm";
const string IO = "/io";

int Monitor::CpuTable::_getUpTime()
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

double MonitorService::_calculateCpuTime(Monitor::CpuTable table)
{
    double utime = table.getUTime() / CLK_TCK;
    double stime = table.getSTime() / CLK_TCK;
    double startTime = table.getStartTime() / CLK_TCK;
    double elapsedTime = table.getUpTime() - startTime;
    double cpuRunTime = ((utime + stime) * 100) / elapsedTime;
    cpuRunTime = cpuRunTime * (1 + (table.getNiceTime() / MAX_NICE_VALUE));
    return cpuRunTime;
}

void MonitorService::_getProcessIds()
{
    int pid = -1;
    DIR *dir = opendir(PROC.c_str());
    dirent *entry = NULL;

    if (dir == NULL)
    {
        AgentUtils::writeLog(INVALID_PATH + PROC, FAILED);
        return;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            try
            {
                pid = std::stoi(entry->d_name);
                this->_processIds.push_back(pid);
            }
            catch (const std::exception &e)
            {
            }
        }
    }
    return;
}

string MonitorService::_getProcesNameById(const unsigned int processId)
{
    string processName;
    string path = PROC + std::to_string(processId) + COMM;
    std::ifstream file(path);

    if (file.is_open())
    {
        std::getline(file, processName);
        file.close();
        return processName;
    }

    return "";
}

int MonitorService::_saveLog(const string path, vector<Monitor::Data> logs, vector<string> columns)
{
    string hostName;
    AgentUtils::getHostName(hostName);
    Monitor::SYS_PROPERTIES properties = getSystemProperties();
    Json::Value props;
    props["CpuMemory"] = properties.cpu;
    props["RamMeomry"] = properties.ram;
    props["DiskMemory"] = properties.disk;
    properties = getAvailedSystemProperties();
    Json::Value availedProps;
    availedProps["CpuMemory"] = properties.cpu;
    availedProps["RamMeomry"] = properties.ram;
    availedProps["DiskMemory"] = properties.disk;
    fstream file(path, std::ios::app);
    Json::Value jsonData;
    Json::StreamWriterBuilder writerBuilder;
    if (!file)
    {
        AgentUtils::writeLog(FWRITE_FAILED + path, FAILED);
        return FAILED;
    }
    if ((int)columns.size() != 5)
    {
        AgentUtils::writeLog("Expect 5 attribute names for monitoring, given " + std::to_string(columns.size()), FAILED);
        return FAILED;
    }
    jsonData["DeviceTotalSpace"] = props;
    jsonData["DeviceUsedSpace"] = availedProps;
    jsonData["TimeGenerated"] = AgentUtils::getCurrentTime();
    jsonData["Source"] = hostName;
    jsonData["OrgId"] = 12345;
    jsonData["ProcessObjects"] = Json::Value(Json::arrayValue);
    for (Monitor::Data data : logs)
    {
        Json::Value jsonLog;
        jsonLog[columns[0]] = std::stoi(data.getProcessId());
        jsonLog[columns[1]] = data.getProcessName();
        jsonLog[columns[2]] = std::stod(data.getCpuTime());
        jsonLog[columns[3]] = std::stod(data.getMemUsage());
        jsonLog[columns[4]] = std::stod(data.getDiskUsage());
        jsonData["ProcessObjects"].append(jsonLog);
    }

    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(jsonData, &file);

    file.close();
    AgentUtils::writeLog(FWRITE_SUCCESS + path, SUCCESS);
    return SUCCESS;
}

int MonitorService::getData(const string writePath, vector<string> columns)
{
    AgentUtils::writeLog("Request for collecting Monitor Logs", DEBUG);
    vector<Monitor::Data> parent;
    _getProcessIds();
    for (int i = 0; i < (int)this->_processIds.size(); i++)
    {
        int processId = this->_processIds.at(i);
        Monitor::CpuTable table = _readProcessingTimeById(processId);
        string processName = _getProcesNameById(processId);
        double cpuTime = _calculateCpuTime(table);
        double memUsage = _getMemoryUsage(processId);
        double diskUsage = _getDiskUsage(processId);
        Monitor::Data child(std::to_string(processId), processName, std::to_string(cpuTime), std::to_string(memUsage), std::to_string(diskUsage));
        parent.push_back(child);
    }
    AgentUtils::writeLog("Monitor Log collected", INFO);
    return _saveLog(writePath, parent, columns);
}

double MonitorService::_getMemoryUsage(const unsigned int processId)
{
    unsigned long size, resident, shared, text, lib, data, dt;
    double memoryPercentage = -1.0;
    string line;
    string path = PROC + std::to_string(processId) + MEMORYDATA;

    long pageSize = sysconf(_SC_PAGESIZE);
    long totalMemory = sysconf(_SC_PHYS_PAGES) * pageSize;

    std::ifstream file(path);
    if (!file)
    {
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return memoryPercentage;
    }
    std::getline(file, line);
    std::istringstream iss(line);

    if (!(iss >> size >> resident >> shared >> text >> lib >> data >> dt))
    {
        AgentUtils::writeLog("Failed to parse memory statistics.", FAILED);
        return memoryPercentage;
    }

    memoryPercentage = 100.0 * resident * pageSize / totalMemory;

    return memoryPercentage;
}

Monitor::CpuTable MonitorService::_readProcessingTimeById(const unsigned int processId)
{
    string path = PROC + std::to_string(processId) + CPUDATA;
    vector<string> stats;
    string line, word;
    fstream file;

    file.open(path, std::ios::in);
    if (!(file.is_open()))
    {
        Monitor::CpuTable emptyTable;
        AgentUtils::writeLog(FILE_ERROR + path, FAILED);
        return emptyTable;
    }
    std::getline(file, line);
    std::stringstream ss(line);
    while (std::getline(ss, word, ' '))
    {
        stats.push_back(word);
    }
    file.close();
    Monitor::CpuTable table(stoi(stats.at(UITME)), stoi(stats.at(STIME)), stoi(stats.at(CUTIME)), stoi(stats.at(CSTIME)), stoi(stats.at(NICETIME)), stoi(stats.at(START_TIME)));
    return table;
}

double MonitorService::_getDiskUsage(const unsigned int processId)
{
    double diskUsage = -1.0;
    int i = 0;
    string line;
    string path = PROC + std::to_string(processId) + IO;
    std::fstream file(path, std::ios::in);

    if (!file.is_open())
    {
        AgentUtils::writeLog("Process does not exist with this id : " + std::to_string(processId), FAILED);
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

    return diskUsage;
}

Monitor::SYS_PROPERTIES MonitorService::getSystemProperties()
{
    AgentUtils::writeLog("Request for collecting availed system properties started...", INFO);
    struct Monitor::SYS_PROPERTIES properties;
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
    return properties;
}

Monitor::SYS_PROPERTIES MonitorService::getAvailedSystemProperties()
{
    AgentUtils::writeLog("Request for collecting availed system properties started..", INFO);
    struct Monitor::SYS_PROPERTIES properties;
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
    return properties;
}

MonitorService::~MonitorService() {}

int main()
{
    return 0;
}