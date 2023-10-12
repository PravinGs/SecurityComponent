#ifndef APPARMOR_SERVICE
#define APPARMOR_SERVICE
#include "common.hpp"
#include "service/command_service.hpp"

class apparmor_service
{
private:
    const string PROFILE_VALIDATION_COMMAND = "apparmor_profile_syntax_check:apparmor_parser -Tv ";
    const string PROFILE_UPLOAD_COMMAND = "apparmor_profile_kernel_upload:apparmor_parser -r -W ";
    const string PROFILE_DISABLE_COMMAND = "apparmor_profile_disable:apparmor_parser -R ";
    const string PROFILE_ENABLE_COMMAND = "apparmor_profile_enable:apparmor_parser -r ";
    const string APPARMOR_RELOAD = "apparmor_reload:systemctl restart apparmor.service";
    const string APPARMOR_SERVICE_STATUS = "apparmor_service_status:systemctl status apparmor.service";
    const string PROFILE_DISABLE_PATH = "/etc/apparmor.d/disable/";
    const string DELETE_FILE = "delete_file:rm -f ";

private:
    string profile_name;
    ICommand *c_service = nullptr;

private:
    bool process_command(const string &command, Json::Value &json)
    {
        vector<string> response_status;
        int mid = command.find_first_of(':');
        int result = c_service->read_command(command.substr(mid + 1), response_status);
        if (result == SUCCESS)
        {
            string attribute = command.substr(0, mid);
            json[attribute] = Json::Value(Json::arrayValue);
            for (const string &status : response_status)
                json[attribute].append(status);
        }
        return (result == FAILED) ? false : true;
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

    bool is_valid_profile(const string &profile, Json::Value &response_status)
    {
        string local_command = PROFILE_VALIDATION_COMMAND;
        bool result = os::is_file_exist(profile);
        if (result)
        {
            local_command += profile;
            result = process_command(local_command, response_status);
        }
        return result;
    }

    bool upload_profile(const string &profile, Json::Value &response_status)
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

    bool update_profile(const string &profile, Json::Value &response_status)
    {
        return upload_profile(profile, response_status);
    }

    bool enable_profile(const string &profile, Json::Value &response_status)
    {
        string disabled_profile_path = PROFILE_DISABLE_PATH + profile.substr(profile.find_last_of('/') + 1);
        bool result = os::is_file_exist(disabled_profile_path);
        if (!result)
        {
            response_status["enable_profile"] = "No profile name found [ " + PROFILE_DISABLE_PATH + " ] here.";
            return result;
        }
        string local_command = DELETE_FILE + disabled_profile_path;
        result = process_command(local_command, response_status);
        local_command = PROFILE_ENABLE_COMMAND + profile;
        result = process_command(local_command, response_status);
        reload_apparmor(response_status);
        return result;
    }

    bool disable_profile(const string &profile, Json::Value &response_status)
    {

        string LINKER_COMMAND = "ln -s ";

        string local_command = PROFILE_DISABLE_COMMAND;

        bool result = is_valid_profile(profile, response_status);

        if (!result)
            return result;

        LINKER_COMMAND += profile;

        LINKER_COMMAND += " " + PROFILE_DISABLE_PATH;

        result = process_command(LINKER_COMMAND, response_status);

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