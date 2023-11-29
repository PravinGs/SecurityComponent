#pragma once
#include "controller/base_api.hpp"
#include "controller/analysis_controller.hpp"
#include "entity/analysis_entity.hpp"

class analysis_api : public base_api
{
private:
    analysis_entity entity;
public:
    analysis_api(analysis_entity & entity): entity(entity) {}

    int start() { return SUCCESS; }

    int update() { return SUCCESS; }

    int stop() { return SUCCESS; }
    
    ~analysis_api() {}
};


