#ifndef TPM_CONTROLLER
#define TPM_CONTROLLER

#include "comm/client.hpp"
#include "service/config_service.hpp"

class tpm_controller 
{   
    private:
        tls_client client;
        Config config;
        entity_parser parser;
        bool is_tpm_connected;
        bool is_valid_config;
        map<string, map<string, string>> config_table;
    public:
        tpm_controller(const string& config_file): is_tpm_connected(false) 
        {
            is_valid_config = (config.read_ini_config_file(config_file) == SUCCESS) ? true : false;
        }

        void connect()
        {
            conn_entity entity = parser.get_conn_entity(config_table, "client");
            is_tpm_connected = client.connect(entity);
            if (is_tpm_connected)
            {
                agent_utils::write_log("tpm_controller: connect: connected with tpm server", SUCCESS);
            }
            else
            {
                agent_utils::write_log("tpm_controller: connect: failed to connect with tpm controller", FAILED);
            }
        }

        void clear_tpm()
        {       
            
        }

        void change_key()
        {

        }

};

#endif
