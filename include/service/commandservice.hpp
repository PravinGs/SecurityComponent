#ifndef COMMAND_SERVICE
#define COMMAND_SERVICE
#include "agentUtils.hpp"

/*
netstat -tan | grep LISTEN | egrep -v '(127.0.0.1|::1)' | sort
last -n 5
df -P
*/

class ICommand
{
public:
    virtual int readCommand(const string command) = 0;
    virtual ~ICommand() {}
};

class Command : public ICommand
{
private:
    int validateCommand(const string command);

public:
    int readCommand(const string command);
    int readCommand(const string command, vector<string> &logs);
};
#endif