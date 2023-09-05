#ifndef WATCHSERVICE_HPP
#define WATCHSERVICE_HPP

#include "utils/agentUtils.hpp"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

class IWatch
{
public:
    virtual int start(const string watchDirectory, const string backupDirectory) = 0;
    virtual ~IWatch() {}
};

class WatchService : public IWatch
{

public:
    WatchService() = default;
    ~WatchService();

    int start(const string watchDirectory, const string backupDirectory);
};

#endif