#ifndef RQUEUE_H
#define RQUEUE_H
#pragma once

#include "agent_utils.hpp"

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class Handler : public AMQP::LibEvHandler
{
private:
    string caKey;
    string clientCert;
    string clientKey;

public:
    Handler(struct ev_loop *loop, string caKey, string clientCert, string clientKey) : AMQP::LibEvHandler(loop)
    {
        this->caKey = caKey;
        this->clientCert = clientCert;
        this->clientKey = clientKey;
    }

    void onAttached(AMQP::TcpConnection *connection) {}

    void onConnected(AMQP::TcpConnection *connection) {}

    bool onSecuring(AMQP::TcpConnection *connection, SSL *ssl)
    {
        if (SSL_CTX_load_verify_locations(SSL_get_SSL_CTX(ssl), caKey.c_str(), nullptr) != 1)
        {
            agent_utils::write_log("Failed to load CA certificates: " + caKey, FAILED);
            return false;
        }
        if (SSL_use_certificate_file(ssl, clientCert.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("Failed to load client certificate: " + clientCert, FAILED);
            return false;
        }

        if (SSL_use_PrivateKey_file(ssl, clientKey.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("Failed to load client private key: " + clientKey, FAILED);
            return false;
        }

        if (SSL_check_private_key(ssl) != 1)
        {
            agent_utils::write_log("Private key does not match the certificate: ", FAILED);
            return false;
        }

        return true;
    }

    bool onSecured(AMQP::TcpConnection *connection, const SSL *ssl)
    {
        return true;
    }

    void onError(AMQP::TcpState *state, const char *message, bool connected)
    {
        agent_utils::write_log(message, FAILED);
    }
};

/**
 * @endcond
 */

class Queue
{
private:
    struct ev_loop *_loop = EV_DEFAULT;
    Handler *_handler = nullptr;
    AMQP::TcpConnection *_connection = nullptr;

private:
    int _checkConnection(); // update it to bool return type
    int _publish(string fileName, string queue);
    int _parseJSON(string jsonFile, string &jsonString);

public:
    Queue(string caKey, string clientCert, string clientKey);
    int send(string jsonFile, string queue);
    virtual ~Queue();
};

#endif