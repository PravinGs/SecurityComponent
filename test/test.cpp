#include "service/fwservice.hpp"
#include "service/configservice.hpp"
#include "agentUtils.hpp"
#include "controller/logController.hpp"
#include "controller/monitorController.hpp"
#include "service/loganalysis.hpp"
int main()
{
    Timer timer;
    LogAnalysis logAnalysis("/home/krishna/security/Agent/config/test-rules.xml");
    logAnalysis.start("/home/krishna/security/Agent/config/auth.test");
    return 0;
}