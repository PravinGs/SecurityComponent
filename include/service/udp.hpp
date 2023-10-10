#ifndef UDP_QUEUE
#define UDP_QUEUE

#include "common.hpp"

#define MAX_UDP_MSG_SIZE 65507

/*
    Create a UDP socket.
    Bind the socket to the server address
    Wait until the datagrams packets arrived from the client.
    Process the datagram packet and send reply to the client.
*/
/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class UdpQueue
{
private:
    int serverSocket = -1;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];

public:
    int start()
    {
        serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (serverSocket == -1)
            return FAILED; /* Socket creation failed */

        memset(&serverAddr, 0, sizeof(serverAddr));
        memset(&clientAddr, 0, sizeof(clientAddr));

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(UDP_PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        {
            agent_utils::write_log("Error binding socket to port.", FAILED);
            close(serverSocket);
        }
        agent_utils::write_log("Binding successfule");
        return serverSocket;
    }

    void stop()
    {
        if (serverSocket)
        {
            close(serverSocket);
        }
    }

    int getMessage(vector<string> &logs)
    {
        if (serverSocket < 0 && start() < 0)
        {
            return FAILED;
        }
        socklen_t length;
        while (1)
        {
            ssize_t recvBytes = recvfrom(serverSocket, buffer, BUFFER_SIZE, MSG_WAITALL,
                                         (struct sockaddr *)&clientAddr, &length);
            if (recvBytes == -1)
            {
                agent_utils::write_log("Error receiving data.", FAILED);
                close(serverSocket);
                return FAILED;
            }
            buffer[recvBytes] = '\0';
            // string log(buffer);
            logs.push_back(buffer);
        }
        close(serverSocket);
        return SUCCESS;
    }

    int sendMessage(const string message)
    {
        if (serverSocket < 0)
        {
            return FAILED;
        }

        socklen_t clientAddrLen;
        ssize_t sendBytes = sendto(serverSocket, message.c_str(), sizeof(message), 0,
                                   (struct sockaddr *)&clientAddr, clientAddrLen);

        if (sendBytes == -1)
        {
            cerr << "Error sending data." << "\n";
            close(serverSocket);
            return FAILED;
        }
        return SUCCESS;
    }
};
/**
 * @endcond
 */

#endif