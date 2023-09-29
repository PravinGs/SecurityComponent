#include "controller/mainController.hpp"
#include "agentUtils.hpp"


int main()
{
    AgentUtils::syslog_enabled = true;
    AgentUtils::setupLogger();
    if (!AgentUtils::syslog_enabled)
    {
        AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    }
    
    MainController controller(AGENT_CONFIG_DIRECTORY);
    controller.start();
    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    return 0;
}