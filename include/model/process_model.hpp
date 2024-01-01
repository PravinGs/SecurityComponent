#pragma once 

#include "entity.hpp"

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


struct sys_properties {
    double ram;
    double disk;
    double cpu;
};


struct process_data {

    string process_id;

    string process_name;

    string cpu_time;

    string ram_usage;

    string disk_usage;

    process_data(string id, string name, string c_time, string m_sage, string d_usage) 
        : process_id(id), process_name(name), cpu_time(c_time), ram_usage(m_sage), disk_usage(d_usage)
    {}
};

class cpu_table
{
private:
    int utime;
    int stime;
    int cutime;
    int cstime;
    int start_time;
    int nice_time;
    int up_time;
    int cpu_count;
    int get_up_time();

public:
    cpu_table(int utime, int stime, int cutime, int cstime, int nice_time, int start_time)
        : utime(utime), stime(stime), cutime(cutime), cstime(cstime), start_time(start_time), nice_time(nice_time)
    {
        this->up_time = get_up_time();
        this->cpu_count = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }

    cpu_table() {}

    int getUTime() { return utime; }
    int getSTime() { return stime; }
    int getCuTime() { return cutime; }
    int getCsTime() { return cstime; }
    int getStartTime() { return start_time; }
    int getUpTime() { return up_time; }
    int getCpuCount() { return cpu_count; }
    int getNiceTime() { return nice_time; }
};
