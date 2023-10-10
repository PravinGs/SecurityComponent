#ifndef cOMMAND_SERVICE
#define cOMMAND_SERVICE
#include "common.hpp"

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class ICommand
{
public:
    virtual int read_command(const string& command, vector<string> & process_name) = 0;
    virtual ~ICommand() {}
};
/**
 * @endcond
 */

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class command : public ICommand
{
private:
    int validate_command(const string command);

public:
    command(){}
    // int read_command(const string command, const string process_name);
    int read_command(const string& command, vector<string> &logs);
};
/**
 * @endcond
 */


#endif