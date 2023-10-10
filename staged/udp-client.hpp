#ifndef UDP_QUEUE
#define UDP_QUEUE

#include "agent_utils.hpp"

class UdpClient
{
private:
    int clientSocket = -1;
    struct sockaddr_in serverAddress;
    string buffer[BUFFER_SIZE];

public:
    int start()
    {
        clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSocket == -1)
        {
            agent_utils::write_log("Socket creation failed", FAILED);
            return FAILED;
        }
        agent_utils::write_log("Socket created for client");
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(UDP_PORT);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        return clientSocket;
    }

    int sendMessage(string message)
    {
        if (clientSocket < 0 && start() < 0)
        {
            return FAILED;
        }

        ssize_t sendBytes = sendto(clientSocket, message.c_str(), message.size(), MSG_CONFIRM, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (sendBytes == -1)
        {
            agent_utils::write_log("Error sending data.", FAILED);
            close(clientSocket);
            return FAILED;
        }
        return SUCCESS;
    }

    void stop()
    {
        if (clientSocket)
        {
            close(clientSocket);
        }
    }
};

#endif