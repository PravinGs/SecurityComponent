#ifndef ROOT_CHECK
#define ROOT_CHECK
#pragma once

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
        interfaceCheck interfaceCheck;
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
            AgentUtils::writeLog("Starting trojan root check.", INFO);
            if (trojanCheck.check(trojanSourceFile))
            {
                AgentUtils::writeLog("Trojan root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Trojan root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed trojan root check.", INFO);
            AgentUtils::writeLog("Starting sysfiles root check.");
            if (sysCheck.check(sysSourceFile))
            {
                AgentUtils::writeLog("SysFiles root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("Sysfiles root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed sysfiles root check.");
            AgentUtils::writeLog("Starting network interface root check.", INFO);
            if (interfaceCheck.check())
            {
                AgentUtils::writeLog("network interface  root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("network interface root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed network interface root Check.", INFO);
            AgentUtils::writeLog("Starting dev root check.", INFO);
            if (devCheck.check())
            {
                AgentUtils::writeLog("dev root check completed successfully.", SUCCESS);
            }
            else
            {
                AgentUtils::writeLog("dev root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed dev root check.", INFO);
            AgentUtils::writeLog("Starting process root check", INFO);
            AgentUtils::writeLog("This process might take more time");
            if (processCheck.check())
            {
                AgentUtils::writeLog("Process root check completed successfully.", SUCCESS);
            }
            else
            {
                 AgentUtils::writeLog("Process root check failed", FAILED);
            }
            AgentUtils::writeLog("Completed process root check", INFO);
            return SUCCESS;
        }

};

#endif