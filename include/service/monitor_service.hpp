#ifndef LINMONITOR_HPP
#define LINMONITOR_HPP
#pragma once

#include "model/process_model.hpp"
#include "repository/process_repository.hpp"
class IMonitor
{
public:

    virtual int get_monitor_data(const process_entity & entity) = 0;

    virtual sys_properties get_system_properties() = 0;

    virtual sys_properties get_availed_system_properties() = 0;

    virtual ~IMonitor() {}
};

class monitor_service : public IMonitor
{
private:
    vector<std::future<void>> async_tasks;   
    process_repository db; 
private:

    int save_monitor_log(const vector<process_data>& logs);

    string get_proces_name_id(const unsigned int& process_id);

    vector<int> get_processes_id();

    cpu_table read_processing_time_id(const unsigned int& process_id);

    double calculate_cpu_time(cpu_table& table);

    double get_memory_usage(const unsigned int& process_id);

    double get_disk_usage(const unsigned int& process_id);

    process_data create_process_data(int process_id);

public:

    monitor_service() {}

    int get_monitor_data(const process_entity & entity);

    sys_properties get_system_properties();

    sys_properties get_availed_system_properties();

    ~monitor_service();
};

#endif