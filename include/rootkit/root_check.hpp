#ifndef ROOT_CHECK
#define ROOT_CHECK

#include "rootkit/dev_check.hpp"
#include "rootkit/interface_check.hpp"
#include "rootkit/ports_check.hpp"
#include "rootkit/process_check.hpp"
#include "rootkit/trojen_check.hpp"
#include "rootkit/sysfile_check.hpp"

#define TROJAN_SOURCE_FILE "/etc/scl/ids/source/rootkit_trojans.txt"
#define SYS_SOURCE_FILE "/etc/scl/ids/source/rootkit_files.txt"

class RootCheck
{
    
    private:
        TrojenCheck trojanCheck;
        SysCheck sysCheck;
        ProcessCheck processCheck;
        PortCheck portCheck;
        DevCheck devCheck;
        InterfaceCheck interfaceCheck;
        string trojanSourceFile;
        string sysSourceFile;

    public:
        RootCheck()
        {
            this->trojanSourceFile = TROJAN_SOURCE_FILE;
            this->sysSourceFile    = SYS_SOURCE_FILE;
        }

        RootCheck(string trojanSourceFile, string sysSourceFile)
        {
            this->trojanSourceFile = trojanSourceFile;
            this->sysSourceFile    = sysSourceFile; 
        }

        int check()
        {
            AgentUtils::writeLog("Starting Trojan Root Check.");
            if (trojanCheck.check(trojanSourceFile))
            {
                AgentUtils::writeLog("Trojan Root Check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Trojan Root Check failed", FAILED);
            }
            AgentUtils::writeLog("Starting SysFiles Root Check.");
            if (sysCheck.check(sysSourceFile))
            {
                AgentUtils::writeLog("SysFiles Root Check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("SysFiles Root Check failed", FAILED);
            }
            AgentUtils::writeLog("Starting Network Interface Root Check.");
            if (interfaceCheck.check())
            {
                AgentUtils::writeLog("Network Interface  Root Check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Network Interface Root Check failed", FAILED);
            }
            AgentUtils::writeLog("Starting dev Root Check.");
            if (devCheck.check())
            {
                AgentUtils::writeLog("dev Root Check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("dev Root Check failed", FAILED);
            }
            AgentUtils::writeLog("Starting Process Root Check");
            AgentUtils::writeLog("This process might take more time");
            if (processCheck.check())
            {
                AgentUtils::writeLog("Process Root Check completed successfully.", SUCCESS);
            }
            else
            {
                 AgentUtils::writeLog("Process Root Check failed", FAILED);
            }

            return SUCCESS;
        }

};

#endif