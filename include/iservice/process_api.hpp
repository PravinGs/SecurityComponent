#pragma once
#include "controller/base_api.hpp"
#include "controller/monitor_controller.hpp"
#include "entity/process_entity.hpp"

class process_api : public base_api
{
private:
    process_entity entity;
public:
    process_api(process_entity& entity) : entity(entity) {}
    
    ~process_api() {}
    
    int start() { return SUCCESS; }

    int update() { return SUCCESS; }

    int stop() { return SUCCESS; }
};

