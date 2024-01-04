#ifndef TLS_CLIENT_HPP
#define TLS_CLIENT_HPP

#include "model/conn_model.hpp"

class tls_client
{
private:
    SSL *ssl = NULL;
    SSL_CTX *ctx = NULL;
    BIO *sbio = NULL;
    int r;
    int rc = -1;

private:
    SSL_CTX *get_ssl_context(const conn_entity &entity)
    {
        SSL_CTX *ctx = nullptr;

        ctx = SSL_CTX_new(SSLv23_client_method());

        if (!(ctx))
        {
            agent_utils::write_log("tls_client: get_ssl_context: SSL_CTX_new failed", FAILED);
            return ctx;
        }

        if (SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Failed to set min protocol version", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        if (SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Failed to set max protocol version.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        /* Set the CA file location for the server */
        if (SSL_CTX_load_verify_locations(ctx, entity.ca_pem.c_str(), NULL) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Could not set the CA file location.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        /* Load the client's CA file location as well */
        SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(entity.cert_pem.c_str()));

        /* Set the server's certificate signed by the CA */
        if (SSL_CTX_use_certificate_file(ctx, entity.cert_pem.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Could not set the server's certificate.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        /* Set the server's key for the above certificate */
        if (SSL_CTX_use_PrivateKey_file(ctx, entity.key_pem.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Could not set the server's key.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        /* We've loaded both certificate and the key, check if they match */
        if (SSL_CTX_check_private_key(ctx) != 1)
        {
            agent_utils::write_log("tls_client: get_ssl_context: Server's certificate and the key don't match.", FAILED);
            SSL_CTX_free(ctx);
            return nullptr;
        }

        /* We won't handle incomplete read/writes due to renegotiation */
        SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

        /* Specify that we need to verify the client as well */
        SSL_CTX_set_verify(ctx,
                           SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                           NULL);

        /* We accept only certificates signed only by the CA himself */
        SSL_CTX_set_verify_depth(ctx, 1);

        return ctx;
    }

public:
    tls_client() : r(-1), rc(-1)
    {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    }

    int start(const conn_entity &entity)
    {
        if (!(ctx = get_ssl_context(entity)))
        {
            return FAILED;
        }

        if (!(sbio = BIO_new_ssl_connect(ctx)))
        {
            agent_utils::write_log("tls_client: start: Could not get a BIO object from context", FAILED);
            SSL_CTX_free(ctx);
            return FAILED;
        }

        BIO_get_ssl(sbio, &ssl);

        if (BIO_set_conn_hostname(sbio, entity.conn_string.c_str()) != 1)
        {
            agent_utils::write_log("tls_client: start: Could not connection to the server", FAILED);
            BIO_free_all(sbio);
            return FAILED;
        }

        if ((r = SSL_do_handshake(ssl)) != 1)
        {
            agent_utils::write_log("tls_client: start: SSL Handshake failed", FAILED);
            BIO_free_all(sbio);
            return FAILED;
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK)
        {
            agent_utils::write_log("tls_client: start: Verification of handshake failed", FAILED);
            BIO_free_all(sbio);
            return FAILED;
        }

        return SUCCESS;
    }

    int send_client_data(const client_data &data)
    {
        string client_data_buffer(reinterpret_cast<const char *>(&data), sizeof(data));
        char received_buffer[sizeof(server_data)];

        int bytes_sent = SSL_write(ssl, client_data_buffer.c_str(), client_data_buffer.size());

        if (bytes_sent < 0)
        {
            std::cerr << "Error sending data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return bytes_sent;
        }

        int bytes_received = SSL_read(ssl, received_buffer, sizeof(received_buffer));

        if (bytes_received < 0)
        {
            std::cerr << "Error receiving data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return bytes_received;
        }
        else
        {
            server_data s_data = *reinterpret_cast<server_data *>(received_buffer);
            cout << s_data.status << '\n';
            cout << s_data.data << '\n';
        }

        return bytes_received;
    }
};
#endif
