#include "agentUtils.hpp"
#include "rootkit/interface_check.hpp"
#include "rootkit/ports_check.hpp"
#include "rootkit/dev_check.hpp"
#include "rootkit/process_check.hpp"
#include "rootkit/sysfile_check.hpp"

int main() {
    openlog("agent", LOG_INFO | LOG_CONS, LOG_USER);
    
    // InterfaceCheck process;
    // PortCheck port;
    // port.check();
    // DevCheck dev;
    // ProcessCheck process;
    // int result = process.check();
    // cout << "Response is " << result << endl;
    SysCheck sys;
    sys.check("/home/krishna/Build-Libs/ossec-hids/src/rootcheck/db/rootkit_files.txt");
    closelog();
    return 0; 
}

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
