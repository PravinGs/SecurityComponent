#ifndef TLS_SERVER_HPP
#define TLS_SERVER_HPP

#include "common.hpp"
#include "model/entity.hpp"

std::mutex server_mutex;
std::condition_variable condition_var;

class tls_server
{
private:
    const int THREAD_POOL_SIZE = 5;
    std::queue<SSL *> ssl_queue;
    std::queue<client_data> rest_queue;
    std::vector<std::thread> thread_pool;

private:
    void *handle_client(void *arg)
    {
        SSL *client_ssl = (SSL *)arg;
        try
        {
            home(client_ssl);
        }
        catch (const std::exception &e)
        {
            agent_utils::write_log("tls_server: handle_client: Exception during handling client: " + std::string(e.what()), WARNING);
        }
        SSL_free(client_ssl);
        return nullptr;
    }

    void enqueue(SSL *ssl)
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        ssl_queue.push(ssl);
    }

    SSL *dequeue()
    {
        if (ssl_queue.empty())
        {
            cout << "Queue is empty" << '\n';
            return nullptr;
        }
        SSL *ssl = ssl_queue.front();
        ssl_queue.pop();
        return ssl;
    }

    int get_socket(int port_num)
    {
        struct sockaddr_in sin;
        int sock, val;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            agent_utils::write_log("tls_server: get_socket: cannot create a socket", FAILED);
            return -1;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
        {
            agent_utils::write_log("tls_server: get_socket: could not set SO_REUSEADDR on the socket", FAILED);
            close(sock);
            return -1;
        }

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port_num);

        if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            agent_utils::write_log("tls_server: get_socket: could not bind the socket", FAILED);
            close(sock);
            return -1;
        }

        if (listen(sock, SOMAXCONN) < 0)
        {
            agent_utils::write_log("tls_server: get_socket: failed to listen on this socket", FAILED);
            close(sock);
            return -1;
        }

        return sock;
    }

    SSL_CTX *get_ssl_context(const conn_entity &entity)
    {
        SSL_CTX *ctx = nullptr;

        ctx = SSL_CTX_new(TLS_server_method());

        if (!(ctx))
        {
            agent_utils::write_log("tls_server: get_ssl_context: SSL_CTX_new failed", FAILED);
            return ctx;
        }

        if (SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Failed to set min protocol version", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Failed to set max protocol version.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_load_verify_locations(ctx, entity.ca_pem.c_str(), NULL) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Could not set the CA file location.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(entity.ca_pem.c_str()));

        if (SSL_CTX_use_certificate_file(ctx, entity.cert_pem.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Could not set the server's certificate.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, entity.key_pem.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Could not set the server's key.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_check_private_key(ctx) != 1)
        {
            agent_utils::write_log("tls_server: get_ssl_context: Server's certificate and the key don't match.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

        SSL_CTX_set_verify(ctx,
                           SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                           NULL);

        SSL_CTX_set_verify_depth(ctx, 1);

        return ctx;
    }

    void thread_function()
    {
        while (true)
        {
            SSL *ssl = nullptr;
            {
                std::unique_lock<std::mutex> lock(server_mutex);
                condition_var.wait(lock, [&]()
                                   { return !ssl_queue.empty(); });
                ssl = dequeue();
            }
            if (ssl != nullptr)
            {
                handle_client(ssl);
            }
            else
            {
                agent_utils::write_log("tls_server: thread_functin: ssl is null", FAILED);
            }
        }
    }

    void home(SSL *ssl)
    {
        server_data s_data;
        s_data.status = 1;
        std::strncpy(s_data.data, "ok", 2);
        s_data.data[sizeof(s_data.data) - 1] = '\0';
        string server_data_buffer(reinterpret_cast<const char *>(&s_data), sizeof(s_data));
        char received_buffer[sizeof(client_data)];

        int bytes_received = SSL_read(ssl, received_buffer, sizeof(received_buffer));

        if (bytes_received < 0)
        {
            std::cerr << "Error receiving data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return;
        }
        else
        {
            client_data c_data = *reinterpret_cast<client_data *>(received_buffer);
            cout << c_data.client_id << '\n';
            // once the processing queue received signal from one of the application
            // if data is valid.
            // it should send the data the rest_service from here.
        }
        
        //This write should not be a block thread, because the rest service might take some time to complete,
        // It should not block the plugin applicaiton execution.
        int bytes_sent = SSL_write(ssl, server_data_buffer.c_str(), server_data_buffer.size());

        if (bytes_sent < 0)
        {
            std::cerr << "Error sending data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return;
        }

        return;
    }

public:
    tls_server() : thread_pool(THREAD_POOL_SIZE)
    {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    }

    void start(const conn_entity &entity)
    {
        struct sockaddr_in client_addr;
        SSL_CTX *ctx = nullptr;
        SSL *ssl = nullptr;
        int listen_fd;
        int client_fd;
        if (entity.port < 1 || entity.port > 65535)
        {
            agent_utils::write_log("tls_server: start: invalid port number " + std::to_string(entity.port), FAILED);
            return;
        }

        ctx = get_ssl_context(entity);
        if (!(ctx))
        {
            return;
        }

        listen_fd = get_socket(entity.port);

        if (listen_fd < 0)
        {
            return;
        }

        for (int i = 0; i < THREAD_POOL_SIZE; i++)
        {
            thread_pool.emplace_back([this]()
                                     { thread_function(); });
        }

        while (true)
        {
            socklen_t client_addr_len = sizeof(client_addr);
            client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_fd < 0)
            {
                agent_utils::write_log("tls_server: start: error accepting incoming connection.", WARNING);
                continue;
            }
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
            std::string conn_address(client_ip);

            std::string conn_port = std::to_string(ntohs(client_addr.sin_port));
            agent_utils::write_log("tls_server: start: New Connection from " + conn_address + " : " + conn_port, DEBUG);

            if (!(ssl = SSL_new(ctx)))
            {
                agent_utils::write_log("tls_server: start: error creating SSL object for client.", WARNING);
                continue;
            }

            if (SSL_set_fd(ssl, client_fd) != 1)
            {
                agent_utils::write_log("tls_server: start: error aatching ssl object to client socket.", WARNING);
                SSL_free(ssl);
                continue;
            }

            if (SSL_accept(ssl) <= 0)
            {
                string error = ERR_error_string(ERR_get_error(), NULL);
                agent_utils::write_log("tls_server: start: error performing ssl handshake: " + error, WARNING);
                SSL_free(ssl);
                continue;
            }
            enqueue(ssl);
            condition_var.notify_one();
        }
        for (int i = 0; i < THREAD_POOL_SIZE; i++)
        {
            if (thread_pool[i].joinable())
            {
                thread_pool[i].join();
            }
        }
    }
};

#endif