#pragma once 
#include "controller/base_api.hpp"
#include "entity/log_collector.hpp"

class log_service: public base_api
{
    public:
        int start(std::shared_ptr<void> data)
        {
            auto model = std::static_pointer_cast<log_entity>(data.get());  
        }
}