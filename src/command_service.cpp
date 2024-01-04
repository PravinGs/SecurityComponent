#include "service/command_service.hpp"

vector<string> whitelist = {"netstat -tan | grep LISTEN | egrep -v '(127.0.0.1|::1)' | sort", "df -P", "last -n 5"};


int command::validate_command(const string command)
{
    int result = FAILED;
    for (string s : whitelist)
    {
        if (strcmp(s.c_str(), command.c_str()) == 0)
        {
            result = SUCCESS;
            break;
        }
    }
    return result;
}

int command::read_command(const string& command, vector<string> &logs)
{
    // if (validate_command(command) == FAILED)
    //     return FAILED;

    FILE *process = popen(command.c_str(), "r");

    char buffer[BUFFER_SIZE];

    if (!process)
    {
        agent_utils::write_log("command: read_commmand: failed to open this process for " + command, FAILED);
        return FAILED;
    }

    while (!feof(process))
    {
        if (fgets(buffer, sizeof(buffer), process) != nullptr)
        {
            logs.push_back(buffer);
        }
    }
    pclose(process);

    return SUCCESS;
}