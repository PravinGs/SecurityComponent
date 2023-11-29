#pragma once 
#include "controller/base_api.hpp"
#include "entity/log_entity.hpp"

class log_api: public base_api
{
    private:
        log_entity entity;
        
    public:
        log_api(const log_entity& entity) : entity(entity) {}

        int start () override
        {
            return SUCCESS;
        }

        int update() override { return SUCCESS; }

        int stop() override { return SUCCESS; }
};