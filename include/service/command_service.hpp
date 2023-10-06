#ifndef COMMAND_SERVICE
#define COMMAND_SERVICE
#include "common.hpp"

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class ICommand
{
public:
    virtual int readCommand(const string& command, vector<string> & processName) = 0;
    virtual ~ICommand() {}
};
/**
 * @endcond
 */

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class Command : public ICommand
{
private:
    int validateCommand(const string command);

public:
    Command(){}
    // int readCommand(const string command, const string processName);
    int readCommand(const string& command, vector<string> &logs);
};
/**
 * @endcond
 */

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class Threshold
{
public:
    static int MinFailedLoginAttempts;
    static int MaxFailedLoginAttempts;
};
/**
 * @endcond
 */
#endif