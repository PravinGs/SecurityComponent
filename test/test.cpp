#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"

int main()
{
    IFService *service = new FService();
    IniConfig config;
    map<string, map<string, string>> table;
    config.readConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    service->start(table);
    delete service;
    return 0;
}