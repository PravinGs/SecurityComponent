#include "model/entity.hpp"

#define CLIENT_QUEUE_SIZE 10

typedef struct client_data client_data;
typedef struct server_data server_data;

struct client_data
{
    int is_alive;
    char client_id[25];
    char data[100];

    client_data() : is_alive(0) {}
};

struct server_data
{
    int status;
    char data[100];
    server_data() : status(0) {}
};

class connection
{
public:
    static client_data build_client_data(const string &client_id, const string &data)
    {
        client_data client;
        client.is_alive = 1;
        std::strncpy(client.client_id, client_id.c_str(), sizeof(client.client_id) - 1);
        client.client_id[sizeof(client.client_id) - 1] = '\0';
        std::strncpy(client.data, data.c_str(), sizeof(client.data) - 1);
        client.data[sizeof(client.data) - 1] = '\0';
        return client;
    }
};