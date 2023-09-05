#ifndef PORT_CHCEK
#define PORT_CHECK
#pragma once

#include "agentUtils.hpp"

#define NETSTAT "netstat -an | grep \"^%s\" | " \
                "grep \"[^0-9]%d \" > /dev/null 2>&1"

class PortCheck
{
    private:
        int errors;
        int total;
        int total_ports_tcp[65535 + 1];
        int total_ports_udp[65535 + 1];

    public:

        int runNetStat(int protocol, int port)
        {
            int ret;
            char nt[OS_SIZE_1024 + 1];

            if (protocol == IPPROTO_TCP) {
                snprintf(nt, OS_SIZE_1024, NETSTAT, "tcp", port);
            } else if (protocol == IPPROTO_UDP) {
                snprintf(nt, OS_SIZE_1024, NETSTAT, "udp", port);
            } else {
                AgentUtils::writeLog("Netstat error (wrong protocol)", FAILED);
                return (0);
            }

            ret = system(nt);

            if (ret == 0) {
                return (1);
            } else if (ret == 1) {
                return (0);
            }
            return (1);
        }

        int connPort(int protocol, int port)
        {
            int rc = 0;
            int ossock;
            struct sockaddr_in server;
            struct sockaddr_in6 server6;

            if (protocol == IPPROTO_UDP) {
                if ((ossock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                    return (0);
                }
            } else if (protocol == IPPROTO_TCP) {
                if ((ossock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                    return (0);
                }
            } else {
                return (0);
            }

            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            server.sin_addr.s_addr = htonl(INADDR_ANY);

            /* If we can't bind, it means the port is open */
            if (bind(ossock, (struct sockaddr *) &server, sizeof(server)) < 0) {
                rc = 1;
            }

            /* Setting if port is open or closed */
            if (protocol == IPPROTO_TCP) {
                total_ports_tcp[port] = (char) rc;
            } else {
                total_ports_udp[port] = (char) rc;
            }

            close(ossock);

            /* repeat for IPv6 */
            if (protocol == IPPROTO_UDP) {
                if ((ossock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                    return(0);
                }
            } else if (protocol == IPPROTO_TCP) {
                if ((ossock = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                    return(0);
                }
            }


            memset(&server6, 0, sizeof(server6));
            server6.sin6_family = AF_INET6;
            server6.sin6_port = htons( port );
            memcpy(&server6.sin6_addr.s6_addr, &in6addr_any, sizeof in6addr_any);


            /* If we can't bind, it means the port is open */
            if(bind(ossock, (struct sockaddr *) &server6, sizeof(server6)) < 0) {
                rc = 1;
            }

            /* Setting if port is open or closed */
            if(protocol == IPPROTO_TCP) {
                total_ports_tcp[port] = rc;
            } else {
                total_ports_udp[port] = rc;
            }

            close(ossock);

            return (rc);
        }

        void test_ports(int protocol)
        {
            int i;

            for (i = 0; i <= 65535; i++) {
                total++;
                if (connPort(protocol, i)) {
                    /* Check if we can find it using netstat. If not,
                    * check again to see if the port is still being used.
                    */
                    if (runNetStat(protocol, i)) {
                        continue;
                    }

                    if (!runNetStat(protocol, i) && connPort(protocol, i)) {
                        char op_msg[OS_SIZE_1024 + 1];

                        errors++;

                        snprintf(op_msg, OS_SIZE_1024, "Port '%d'(%s) hidden. "
                                "Kernel-level rootkit or trojaned "
                                "version of netstat.", i,
                                (protocol == IPPROTO_UDP) ? "udp" : "tcp");

                        AgentUtils::writeLog(op_msg, CRITICAL);
                    }
                }

                if ((errors) > 20) {
                    char op_msg[OS_SIZE_1024 + 1];

                    snprintf(op_msg, OS_SIZE_1024, "Excessive number of '%s' ports "
                            "hidden. It maybe a false-positive or "
                            "something really bad is going on.",
                            (protocol == IPPROTO_UDP) ? "udp" : "tcp" );
                    AgentUtils::writeLog(op_msg, CRITICAL);
                    return;
                }
            }

        }

        int check()
        {
            int i = 0;

            while (i <= 65535) {
                total_ports_tcp[i] = 0;
                total_ports_udp[i] = 0;
                i++;
            }

            /* Test both TCP and UDP ports */
            test_ports(IPPROTO_TCP);
            test_ports(IPPROTO_UDP);

            if (errors == 0) {
                char op_msg[OS_SIZE_1024 + 1];

                snprintf(op_msg, OS_SIZE_1024, "No kernel-level rootkit hiding any port."
                        "\n      Netstat is acting correctly."
                        " Analyzed %d ports.", total);
                AgentUtils::writeLog(op_msg, SUCCESS);
            }

            for (int port = 0; port < 65535; port++)
            {
                if (total_ports_tcp[port])
                {
                    cout << "port : " << port << " status : " << total_ports_tcp[port]  << endl;
                }
            }
            return SUCCESS;
        }
};

#endif