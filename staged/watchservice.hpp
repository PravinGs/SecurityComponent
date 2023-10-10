#ifndef WATCHSERVICE_HPP
#define WATCHSERVICE_HPP

#include "agent_utils.hpp"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class IWatch
{
public:
    virtual int start(const string watchDirectory, const string backupDirectory) = 0;
    virtual ~IWatch() {}
};

/**
 * @endcond
 */

/**
 * @cond HIDE_THIS_CLASS
 * @brief This class is for internal use only and should not be documented.
 */
class WatchService : public IWatch
{

public:
    WatchService() = default;
    ~WatchService();

    int start(const string watchDirectory, const string backupDirectory);
};

/**
 * @endcond
 */

#endif