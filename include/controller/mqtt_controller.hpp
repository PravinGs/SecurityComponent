#ifndef MOSQUITTO
#define MOSQUITTO

#include "service/apparmor_service.hpp"
#include "service/mqtt_service.hpp"

typedef struct mqtt_connection_props mqtt_connection_props;

struct mqtt_connection_props
{
    int port;
    int connectio_alive;
    string client_id;
    string topic;
    string server_address;
    string ca_pem;
    string client_cert_pem;
    string client_key_pem;

    mqtt_connection_props():port(0), connectio_alive(60){}
};

class mqtt_controller
{   
    private:

        SSL_CTX * ssl_context = nullptr;
        mosquitto * mosq = nullptr;
        const int PEER_VERIFICATION = 1;
        bool is_connection_alive = false; 
        int qos = 0;
    private:
        mqtt_controller()
        {

        }
        int init()
        {
            mosquitto_lib_init();
            SSL_library_init();
            SSL_load_error_strings();
            mosq = mosquitto_new("champ", true, nullptr);

            if (!mosq) {
                agent_utils::write_log("Unable to create Mosquitto client instance.", ERROR);
                return 1;
            }

            mosquitto_message_callback_set(mosq, message_callback);
        }

        void create_ssl_context(const char* ca_pem, const char* client_cert_pem, const char * client_key_pem)
        {
            if (ssl_context == nullptr)
            {
                ssl_context = SSL_CTX_new(SSLv23_client_method());

                if (!ssl_context)
                {
                    agent_utils::write_log("Cannot create a client context", FAILED);
                    return;
                }
            }

            if (SSL_CTX_set_min_proto_version(ssl_context, TLS1_3_VERSION) != 1)
            {
                agent_utils::write_log("Failed to set min protocol version.", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            if (SSL_CTX_set_max_proto_version(ssl_context, TLS1_3_VERSION) != 1)
            {
                agent_utils::write_log("Failed to set max protocol version.", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            if (SSL_CTX_load_verify_locations(ssl_context, ca_pem, NULL) != 1)
            {
                agent_utils::write_log("Cannot load client's CA file", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            /* Load the client's certificate */
            if (SSL_CTX_use_certificate_file(ssl_context, client_cert_pem, SSL_FILETYPE_PEM) != 1)
            {
                agent_utils::write_log("Cannot load client's certificate file", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            /* Load the client's key */
            if (SSL_CTX_use_PrivateKey_file(ssl_context, client_key_pem, SSL_FILETYPE_PEM) != 1)
            {
                agent_utils::write_log("Cannot load client's key file", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            /* Verify that the client's certificate and the key match */
            if (SSL_CTX_check_private_key(ssl_context) != 1)
            {
                agent_utils::write_log("Client's certificate and key don't match", FAILED);
                SSL_CTX_free(ssl_context);
                return;
            }

            SSL_CTX_set_mode(ssl_context, SSL_MODE_AUTO_RETRY);

            /* Specify that we need to verify the server's certificate */
            SSL_CTX_set_verify(ssl_context, SSL_VERIFY_PEER, NULL);

            /* We accept only certificates signed only by the CA himself */
            SSL_CTX_set_verify_depth(ssl_context, 1);

        }

        void handle_error(int error)
        {
            if (error == MOSQ_ERR_INVAL)
            {
                agent_utils::write_log("the input parameters(certificates) are invalid", FAILED);
            }else if (error == MOSQ_ERR_NOMEM)
            {
                agent_utils::write_log("Out of memory condition occurred", FAILED);
            }
        }

        void mosq_cleanup()
        {
            mosquitto_disconnect(mosq);
            mosquitto_destroy(mosq);
            mosquitto_lib_cleanup();
        }

        void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) 
        {
            if (message->payloadlen) {
                std::string data ((char *)message->payload, message->payloadlen);
                handle_message(data);
                std::cout << "Message: " << std::string((char *)message->payload, message->payloadlen) << "\n";
            } else {
                std::cout << "Empty message received on topic: " << message->topic << "\n";
            }
        }

        int create_mqtt_connection(const struct & mqtt_connection_props)
        {
            int result = MOSQ_ERR_SUCCESS;
            result = mosquitto_tls_set(mosq, mqtt_connection_props.ca_pem, nullptr, mqtt_connection_props.client_cert_pem, mqtt_connection_props.client_key_pem, nullptr);
            
            if (result != MOSQ_ERR_SUCCESS) {
                handle_error(result);
                return result;
            }
            mosquitto_tls_opts_set(mosq, PEER_VERIFICATION, nullptr, nullptr);
            if (result != MOSQ_ERR_SUCCESS) {
                handle_error(result);
                return result;
            }

            result = mosquitto_connect(mosq, mqtt_connection_props.server_address.c_str(), mqtt_connection_props.port, mqtt_connection_props.connection_alive);

            if (result != MOSQ_ERR_SUCCESS) {
                handle_error(result);
                mosq_cleanup();
                return result;
            }

            result = mosquitto_subscribe(mosq, nullptr, mqtt_connection_props.topic.c_str(), qos);

            if (result != MOSQ_ERR_SUCCESS) {
                handle_error(result);
                mosq_cleanup();
                return result;
            }
            is_connection_alive = true;
            return result;
        }


    public:

        void set_qos(const int qos)
        {
            this->QOS = qos;
        }
        
        void start(const struct & mqtt_connection_props)
        {
            int result = init();
            
            if (result != MOSQ_ERR_SUCCESS ) return;

            result = create_mqtt_connection(mqtt_connection_props);

            if (result != MOSQ_ERR_SUCCESS) return;

            while(is_connection_alive)
            {
                result = mosquitto_loop(mosq, -1, 1);
                if (result) {
                    std::cerr << "Error: Unable to loop and handle messages. Return code: " << rc << "\n";
                    break;
                }
            }
            
        }
        
        void handle_message(const string & json_string)
        {
            Json::CharReaderBuilder readerBuilder;
            Json::CharReader* reader = readerBuilder.newCharReader();
            Json::Value root;

            // Parse the JSON string
            std::istringstream iss(json_string);
            std::string errs;
            Json::parseFromStream(reader, iss, &root, &errs);

            if (!errs.empty()) {
                std::cerr << "Error: Failed to parse JSON: " << errs << "\n";
                return 1;
            }

            // Access the array under "ApparmorStatus"
            const Json::Value& apparmorStatusArray = root["ApparmorStatus"];

            // Ensure it's an array
            if (!apparmorStatusArray.isArray()) {
                std::cerr << "Error: ApparmorStatus is not an array\n";
                return 1;
            }

            // Print the elements of the array
            for (const auto& status : apparmorStatusArray) {
                std::cout << status.asString() << "\n";
            }
            return;
        }

};

#endif