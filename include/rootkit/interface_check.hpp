#ifndef INTERFACE_ROOT_CHECK
#define INTERFACE_ROOT_CHECK

#pragma once

#include "common.hpp"

#define PROMISCOUS "ifconfig %s | grep PROMISC > /dev/null 2>&1"

/*
    Commmand :: "ifconfig %s | grep PROMISC > /dev/null 2>&1" :: To identify the interface is in promiscuous mode
    steps    :: Get all the available interfaces.
*/

class interface_check
{
private:
    vector<string> promiscuousInterfaces;
    int fd; /* File descriptor */
    int errors;
    int total;
    struct ifreq tmp_str[16];
    struct ifconf _if;
    struct ifreq *_ir;
    struct ifreq *_ifend;
    struct ifreq _ifr;

public:
    int isInterfaceInPromiscuousMode(const std::string& interfaceName)
    {
        char nt[OS_SIZE_1024 + 1];

        snprintf(nt, OS_SIZE_1024, PROMISCOUS, interfaceName.c_str());
        if (system(nt) == 0) {
            return (1);
        }
        return (0);
    }
    
    int check(vector<string> & reports)
    {
        agent_utils::write_log("interface_check: check: checking network interfaces starting", INFO);
        fd = socket(AF_INET, SOCK_DGRAM, 0);

        if (fd < 0)
        {
            agent_utils::write_log("interface_check: check: error checking interfaces (socket)", FAILED);
            return FAILED;
        }

        memset(tmp_str, 0, sizeof(struct ifreq) * 16);
        _if.ifc_len = sizeof(tmp_str);
        _if.ifc_buf = (caddr_t)(tmp_str);

        if (ioctl(fd, SIOCGIFCONF, &_if) < 0)
        {
            close(fd);
            agent_utils::write_log("interface_check: check: error checking interfaces (ioctl)", FAILED);
            return FAILED;
        }

        _ifend = (struct ifreq *) (void *) ((char *)tmp_str + _if.ifc_len);
        _ir = tmp_str;

        for (; _ir < _ifend; _ir++) 
        {
            strncpy(_ifr.ifr_name, _ir->ifr_name, sizeof(_ifr.ifr_name));
            
            // Get information from each interface
            if (ioctl(fd, SIOCGIFFLAGS, (char *)&_ifr) == -1) {
                continue;
            }

            total++;

            if ((_ifr.ifr_flags & IFF_PROMISC)) {
                char op_msg[OS_SIZE_1024];
                // Check if the interface is in promiscuous mode
                if (isInterfaceInPromiscuousMode(_ifr.ifr_name)) {
                    // Assume we have a function to notify about the promiscuous interface.
                    snprintf(op_msg, OS_SIZE_1024, "Interface '%s' in promiscuous"
                         " mode.", _ifr.ifr_name);
                }
                else
                {
                    snprintf(op_msg, OS_SIZE_1024, "Interface '%s' in promiscuous"
                         " mode, but ifconfig is not showing it"
                         "(probably trojaned).", _ifr.ifr_name);
                }
                reports.push_back(_ifr.ifr_name);
                agent_utils::write_log(interface_check: check: op_msg, FAILED);
                errors++;
            }
        }
        close(fd);

        if (errors == 0) {
            agent_utils::write_log("interface_check: check: no problem detected on ifconfig/ifs. analyzed " + std::to_string(total) + " interfaces", INFO);
        }

        return SUCCESS;
    }
};

#endif