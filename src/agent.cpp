#include "controller/mainController.hpp"
#include "agentUtils.hpp"

int main()
{
    openlog("agent", LOG_INFO | LOG_CONS, LOG_USER);
        AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    MainController controller(BASE_CONFIG_DIR);
    controller.start();
        if (AgentUtils::logfp.is_open())
        {
            AgentUtils::logfp.close();
        }
    closelog();
    return 0;
}
