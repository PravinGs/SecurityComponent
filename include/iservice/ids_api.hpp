#pragma once
#include "controller/base_api.hpp"
#include "controller/mqtt_controller.hpp"
#include "entity/ids_entity.hpp"

class ids_api : public base_api
{
private:
    ids_entity entity;
public:
    ids_api(ids_entity& entity) : entity(entity) {}

    int start() { return SUCCESS; }

    int update() { return SUCCESS; }

    int stop() { return SUCCESS; }


    ~ids_api() {}
};

