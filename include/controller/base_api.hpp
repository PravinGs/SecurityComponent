#pragma once


/*
This base_api class will have a common methods for all the security components going to be 
used in this library.
*/

class base_api
{
    public:
        virtual int start(std::shared_ptr<void> data) = 0;
        virtual int run() = 0;
        virtual int stop() = 0;
        // virtual int update(void * data) = 0;
        virtual ~base_api(){}
};