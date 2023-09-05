#include "utils/commandservice.hpp"

vector<string> whitelist = {"netstat -tan | grep LISTEN | egrep -v '(127.0.0.1|::1)' | sort", "df -P", "last -n 5"};

int Command::validateCommand(const string command)
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

int Command::readCommand(const string command, const string processName)
{
    int result = SUCCESS;
    vector<string> logs;
    result = readCommand(command, logs);
    if (result == FAILED)
    {
        return FAILED;
    }
    return SUCCESS;
}

int Command::readCommand(const string command, vector<string> &logs)
{
    if (validateCommand(command) == FAILED)
        return FAILED;

    FILE *process = popen(command.c_str(), "r");

    char buffer[BUFFER_SIZE];

    if (!process)
    {
        AgentUtils::writeLog("Failed to open this process for " + command, FAILED);
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