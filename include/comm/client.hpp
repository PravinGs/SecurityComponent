#ifndef TLS_CLIENT_HPP
#define TLS_CLIENT_HPP

#include "model/entity.hpp"

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

    void start(const conn_entity &entity)
    {
        if (!(ctx = get_ssl_context(entity)))
        {
            return ;
        }

        if (!(sbio = BIO_new_ssl_connect(ctx)))
        {
            agent_utils::write_log("tls_client: start: Could not get a BIO object from context", FAILED);
            SSL_CTX_free(ctx);
            return;
        }

        BIO_get_ssl(sbio, &ssl);

        if (BIO_set_conn_hostname(sbio, entity.conn_string.c_str()) != 1)
        {
            agent_utils::write_log("tls_client: start: Could not connection to the server", FAILED);
            BIO_free_all(sbio);
            return;
        }

        if ((r = SSL_do_handshake(ssl)) != 1)
        {
            agent_utils::write_log("tls_client: start: SSL Handshake failed", FAILED);
            BIO_free_all(sbio);
            return;
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK)
        {
            agent_utils::write_log("tls_client: start: Verification of handshake failed", FAILED);
            BIO_free_all(sbio);
            return;
        }

        home();
    }
    
    int send_string(const std::string &data)
    {
        int bytes_sent = SSL_write(ssl, data.c_str(), data.length());
        if (bytes_sent <= 0)
        {
            std::cerr << "Error sending data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        }
        return bytes_sent;
    }

    void home()
    {
        while (true)
        {
            int data = send_string("client message");
            cout << "Send bytes : " << data << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
};
#endif
