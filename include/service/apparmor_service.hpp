#ifndef APPARMOR_SERVICE
#define APPARMOR_SERVICE

#include "common.hpp"
#include "service/command_service.hpp"

class apparmor_service
{
    private:
        const string PROFILE_VALIDATION_COMMAND = "apparmor_profile_syntax_check:apparmor_parser -Tv ";
        const string PROFILE_UPLOAD_COMMAND     = "apparmor_profile_kernel_upload:apparmor_parser -r -W ";
        const string PROFILE_DELETE_COMMAND     = "apparmor_profile_kernel_delete:apparmor_parser -R ";
        const string APPARMOR_RELOAD            = "apparmor_reload:systemctl restart apparmor.service";
        const string APPARMOR_SERVICE_STATUS    = "apparmor_service_status:systemctl status apparmor.service";
    private:
        string profile_name;
        ICommand *c_service = nullptr;

    private:
        bool process_command(const string& command, Json::Value & json)
        {
            vector<string> response_status;
            int mid = command.find_first_of(':');
            int result = c_service->read_command(command.substr(mid + 1), response_status);

            if (result == SUCCESS)
            {
                string attribute = command.substr(0, mid);
                json[attribute] = Json::Value(Json::arrayValue);
                for (const string& status: response_status)
                    json[attribute].append(status);
            }
            
            return (result == FAILED) ? false : true ;
        }

        bool is_profile_exist(const string& profile)
        {
            fstream fp(profile, std::ios::in | std::ios::binary);
            if (!fp.is_open())
            {
                agent_utils::write_log(FILE_ERROR + profile, CRITICAL);
                return false;
            }
            fp.close();
            return true;
        }

        void reload_apparmor(Json::Value & response_status)
        {
            process_command(APPARMOR_RELOAD, response_status);
            process_command(APPARMOR_SERVICE_STATUS, response_status);
        }
        
    public:
        apparmor_service()
        {
            c_service = new command();
        }

        bool is_valid_profile(const string& profile, Json::Value & response_status)
        {
            string local_command = PROFILE_VALIDATION_COMMAND;
            bool result = is_profile_exist(profile);
            if (result)
            {
                local_command += profile;
                result = process_command(local_command, response_status);
            }
            return result;
        }

        bool upload_profile(const string & profile, Json::Value & response_status)
        {
            string local_command = PROFILE_VALIDATION_COMMAND;
            
            bool result = is_valid_profile(profile, response_status);
            
            if (!result) return result;
            
            local_command += profile;
            
            result = process_command(local_command, response_status); // This will load apparmor profile to the kernel
            
            if (!result) return result;

            reload_apparmor(response_status); // to enable the profile restart the apparmor

            return result;
        }

        bool update_profile(const string & profile, Json::Value & response_status)
        {
            return upload_profile(profile, response_status);
        }

        bool delete_profile(const string & profile, Json::Value & response_status)
        {
            string local_command = PROFILE_DELETE_COMMAND;

            bool result = is_valid_profile(profile, response_status);
            
            if (!result) return result;

            local_command += profile;

            result = process_command(local_command, response_status);

            reload_apparmor(response_status);
            
            return result;
        }
        
        ~apparmor_service()
        {
            delete c_service;
        }
    
};

#endif