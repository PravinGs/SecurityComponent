#ifndef NO_DEBUG
#define NO_DEBUG
#endif
// // #include "service/fwservice.hpp"
// // #include "service/configservice.hpp"
// // #include "service/rqueue.hpp"
#include "common.hpp"
#include "service/root_check.hpp"
// // #include "service/commandservice.hpp"
// // #include "controller/logController.hpp"
// // #include "controller/monitorController.hpp"
// // #include "service/loganalysis.hpp"
// // #include "controller/monitorController.hpp"

// // // int main()
// // // {
// // //     Command command;
// // //     vector<string> logs;
// // //     command.readCommand("aa-status",logs);
// // //     for(string line: logs){ cout << line << "\n";}
// // // }
int main()
{
    openlog("agent", LOG_INFO|LOG_CONS, LOG_USER);
    AgentUtils::syslog_enabled = true;
    AgentUtils::setupLogger();
    if (!AgentUtils::syslog_enabled)
    {
        AgentUtils::logfp.open(LOG_PATH, std::ios::app);
    }
    RootCheck check;
    int result = check.start();
    if (result == SUCCESS){
        cout << "Rootcheck success" << "\n";
    }

    if (AgentUtils::logfp.is_open())
    {
        AgentUtils::logfp.close();
    }
    closelog();
    return 0;
}