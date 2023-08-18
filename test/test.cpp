// #include "service/configservice.hpp"
// #include "service/loganalysis.hpp"
// #include "controller/logController.hpp"

// #include "service/reportgeneration.hpp"
#include "service/configservice.hpp"
#include "service/loganalysis.hpp"

int main()
{
    LogAnalysis analysis("/home/krishna/security/rules/syslog_rules.xml");
    if (analysis.start("/etc/scl/log/archives/2023/Aug/3-syslog") == SUCCESS)
    {
        cout << "SUCCESS" << endl;
    }

    // IniConfig config;
    // string path = "/home/krishna/security/rules/syslog_rules.xml";
    // map<string, map<int, AConfig>> table;

    // if (config.readRuleConfig(path, table) == SUCCESS)
    // {
    //     cout << "Executed successfully" << endl;
    // }
    // else
    // {
    //     cout << "Failed" << endl;
    // }

    // for (const auto& group: table)
    // {
    //     cout << "Group : " << group.first << endl;
    //     for (const auto& rule: group.second)
    //     {
    //         AConfig config = rule.second;
    //         cout << "Rule Id: " << config.id << endl;
    //         cout << "Rule Description: " << config.description << endl;
    //     }
    // }
    // Report report;
    // report.generateFromFile("/home/krishna/security/3-auth");
    // string rulePath = "/home/krishna/security/Agent/config/rules.config";
    // LogAnalysis analysis(rulePath);
    // analysis.start("/home/krishna/security/Agent/config/syslog.test");
    
    //map<string, map<string, string>> table;

    // IniConfig config;
    // config.readConfigFile("/home/krishna/security/Agent/config/schedule.config", table);
    // LogController controller;
    // OS::CurrentDay = 13;
    // OS::CurrentMonth = 8;
    // OS::CurrentYear = 2023;
    // controller.sysLogManager(table);
    // string sys = analysis.formatSysLog("Aug 14 12:56:53 ubuntu-20 kernel: [ 6890.826682] audit: type=1400 audit(1691998013.892:1923): avc:  denied  { use } for  pid=11792 ");
    // cout << sys << endl;
    // string dpkg = analysis.formatSysLog("2023-07-26 11:48:19 status installed install-info:amd64 6.7.0.dfsg.2-5");
    // cout << dpkg << endl;
    // analysis.start("/home/krishna/security/Agent/config/syslog.test");
    
    return 0;
}

/*
void printLogInfo(LOG_EVENT logInfo)
    {
        cout << "size : " << logInfo.size << endl;
        cout << "log : " << logInfo.log << endl;
        cout << "format : " << logInfo.format << endl;
        cout << "timestamp : " << logInfo.timestamp << endl;
        cout << "program : " << logInfo.program << endl;
        cout << "user : " << logInfo.user << endl;   
    }
*/

// int main()
// {
//     // IniConfig conf;
//     // string line = "nkfjfdhe";
//     // if (conf.validateText(line))
//     // {
//     //     cout << "Success" << endl;
//     // }
//     // cout << line << endl;
//     // string log     = "error: connect to example.com port 80 failed: Connection refused";
//     // regex error(R"(error: connect to [^\s]+ port \d+ failed: Connection refused)");
//     // smatch matches;
    // if (regex_search(log, matches, error))
    // {
    //     cout << "Found : " << matches.str() << endl; 
    // }else{
    //     cout << "Not Found" << endl;
    // }   
//     return 0;
// }


// #include "agentUtils.hpp"
// // #include "rootkit/interface_check.hpp"
// // #include "rootkit/ports_check.hpp"
// // #include "rootkit/dev_check.hpp"
// // #include "rootkit/process_check.hpp"
// // #include "rootkit/sysfile_check.hpp"
// // #include "rootkit/trojen_check.hpp"

// #include "rootkit/root_check.hpp"

// int main() {
//     openlog("agent", LOG_INFO | LOG_CONS, LOG_USER);
    
//     // InterfaceCheck process;
//     // PortCheck port;
//     // port.check();
//     // DevCheck dev;
//     // ProcessCheck process;
//     // int result = process.check();
//     // cout << "Response is " << result << endl;
//     // SysCheck sys;
//     // TrojenCheck t;
//     // t.check("/home/krishna/Build-Libs/ossec-hids/src/rootcheck/db/rootkit_trojans.txt");
//     // sys.check("/home/krishna/Build-Libs/ossec-hids/src/rootcheck/db/rootkit_files.txt");
//     RootCheck root;
//     root.check();
//     closelog();
//     return 0; 
// }

// #include <iostream>
// #include <cstring>
// #include <unistd.h>
// #include <sys/types.h>
// #include <pwd.h>
// #include <grp.h>

// void print_user_info(const struct passwd* user) {
//     std::cout << "Username: " << user->pw_name << std::endl;
//     std::cout << "UID: " << user->pw_uid << std::endl;
//     std::cout << "GID: " << user->pw_gid << std::endl;
//     std::cout << "Home Directory: " << user->pw_dir << std::endl;
//     std::cout << "Shell: " << user->pw_shell << std::endl;
//     std::cout << std::endl;
// }

// void print_group_info(const struct group* grp) {
//     std::cout << "Group Name: " << grp->gr_name << std::endl;
//     std::cout << "GID: " << grp->gr_gid << std::endl;
//     std::cout << "Members: ";
//     for (char** member = grp->gr_mem; *member != nullptr; ++member) {
//         std::cout << *member << " ";
//     }
//     std::cout << std::endl << std::endl;
// }

// int main() {
//     // Get system users
//     setpwent();
//     std::cout << "System Users:" << std::endl;
//     struct passwd* user;
//     while ((user = getpwent()) != nullptr) {
//         if (user->pw_uid >= 1000) { // Assuming system users have UIDs >= 1000
//             print_user_info(user);
//         }
//     }
//     endpwent();

//     // Get regular users
//     setpwent();
//     std::cout << "Regular Users:" << std::endl;
//     while ((user = getpwent()) != nullptr) {
//         if (user->pw_uid >= 1000 && user->pw_uid <= 65533) { // Assuming regular users have UIDs between 1000 and 65533
//             print_user_info(user);
//         }
//     }
//     endpwent();

//     // Get service users (based on groups starting with 'systemd-')
//     setgrent();
//     std::cout << "Service Users:" << std::endl;
//     struct group* grp;
//     while ((grp = getgrent()) != nullptr) {
//         if (std::strncmp(grp->gr_name, "systemd-", 8) == 0) { // Assuming service users have groups starting with 'systemd-'
//             print_group_info(grp);
//         }
//     }
//     endgrent();

//     return 0;
// }
