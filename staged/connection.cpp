#include "service/connection.hpp"

TlsConnection::TlsConnection(string port, string caKey, string serverCert, string serverKey)
{
    this->_port = port;
    this->_caKey = caKey;
    this->_serverCert = serverCert;
    this->_serverKey = serverKey;
}

SSL_CTX *TlsConnection::_getServerContext(const string caKey, const string serverCert, const string serverKey)
{
    AgentUtils::writeLog("Request to create a server context");
    SSL_CTX *ctx = nullptr;
    SSL_library_init();
    SSL_load_error_strings();

    try
    {
        if (!(ctx = SSL_CTX_new(TLS_server_method())))
        {
            throw std::invalid_argument("SSL_CTX_new failed");
            return nullptr;
        }

        if (SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            throw std::invalid_argument("Failed to set min protocol version");
        }
        if (SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            throw std::invalid_argument("Failed to set max protocol version");
        }

        if (SSL_CTX_load_verify_locations(ctx, caKey.c_str(), NULL) != 1)
        {
            throw std::invalid_argument("Could not set the CA file location");
        }

        SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(caKey.c_str()));

        if (SSL_CTX_use_certificate_file(ctx, serverCert.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            throw std::invalid_argument("Could not set the server's certificate");
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, serverKey.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            throw std::invalid_argument("Could not set the server's key");
        }

        if (SSL_CTX_check_private_key(ctx) != 1)
        {
            throw std::invalid_argument("Server's certificate and the key don't match");
        }

        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL); // Specify that we need to verify the client as well

        SSL_CTX_set_verify_depth(ctx, 1); // We accept only certificates signed only by the CA itself
    }
    catch (const std::exception &e)
    {
        SSL_CTX_free(ctx);
        string error = e.what();
        AgentUtils::writeLog(error, FAILED);
    }
    return ctx;
}

int TlsConnection::_get_socket(int port_num)
{
    AgentUtils::writeLog("Server socket creation");
    int sock, val = 1;
    struct sockaddr_in sin;

    try
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            AgentUtils::writeLog("Cannot create a socket", FAILED);
            return -1;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
        {
            throw std::invalid_argument("Could not set SO_REUSEADDR on the socket.");
        }

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port_num);

        if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            throw std::invalid_argument("Could not bind the socket.");
        }

        if (listen(sock, SOMAXCONN) < 0)
        {
            throw std::invalid_argument("Failed to listen on this socket");
        }

        AgentUtils::writeLog("Server socket created listening to port " + std::to_string(port_num));
    }
    catch (const std::exception &e)
    {
        close(sock);
        sock = -1;
        string error = e.what();
        AgentUtils::writeLog(error, FAILED);
    }
    return sock;
}

int TlsConnection::start()
{
    AgentUtils::writeLog("Starting server...");
    SSL_CTX *ctx = nullptr;
    SSL *ssl = nullptr;
    int port_num, listen_fd, net_fd, rc;
    struct sockaddr_in sin;
    socklen_t sin_len;
    int result = SUCCESS;

    try
    {
        port_num = std::stoi(_port);
        if (port_num < 1 || port_num > 65535)
        {
            AgentUtils::writeLog("Invalid port number: " + _port, FAILED);
            return FAILED;
        }

        if (!(ctx = _getServerContext(_caKey, _serverCert, _serverKey)))
        {
            return FAILED;
        }

        if ((listen_fd = _get_socket(port_num)) < 0)
        {
            throw std::invalid_argument("Unable to listening to the PORT");
        }

        while (true)
        {
            sin_len = sizeof(sin);
            if ((net_fd = accept(listen_fd, (struct sockaddr *)&sin, &sin_len)) < 0)
            {
                std::invalid_argument("Failed to accept connection");
                continue;
            }
            AgentUtils::writeLog("Client connection accepted");

            if (!(ssl = SSL_new(ctx)))
            {
                std::invalid_argument("Could not get an SSL handle from the context.");
                close(net_fd);
                continue;
            }

            SSL_set_fd(ssl, net_fd);

            if ((rc = SSL_accept(ssl)) != 1)
            {
                std::invalid_argument("Could not perform SSL handshake");
                if (rc != 0)
                {
                    SSL_shutdown(ssl);
                }
                SSL_free(ssl);
            }
            string clientAddress = inet_ntoa(sin.sin_addr);
            AgentUtils::writeLog("SSL Handshake successful with [ " + clientAddress + " ] with port " + _port);

            char buffer[1024];
            int received = SSL_read(ssl, buffer, sizeof(buffer));
            if (received <= 0)
            {
                throw std::invalid_argument("Failed to receive response.");
            }
            else
            {
                std::cout << "Received response: " << std::string(buffer, received) << "\n";
            }
            std::string message = "Hello, client!";
            if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
            {
                throw std::invalid_argument("Failed to send message.");
            }
        }

        close(listen_fd);
        SSL_CTX_free(ctx);
    }
    catch (const std::exception &e)
    {
        result = FAILED;
        SSL_CTX_free(ctx);
        string error = e.what();
        AgentUtils::writeLog(error, FAILED);
    }
    return result;
}

TlsConnection::~TlsConnection() {}