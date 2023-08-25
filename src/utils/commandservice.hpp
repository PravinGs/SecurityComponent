#ifndef COMMAND_SERVICE
#define COMMAND_SERVICE
#include "utils/agentUtils.hpp"

class ICommand
{
public:
    virtual int readCommand(const string command, const string processName) = 0;
    virtual ~ICommand() {}
};

class Command : public ICommand
{
private:
    int validateCommand(const string command);

public:
    int readCommand(const string command, const string processName);
    int readCommand(const string command, vector<string> &logs);
};

class Threshold
{
public:
    static int MinFailedLoginAttempts;
    static int MaxFailedLoginAttempts;
};
#endif