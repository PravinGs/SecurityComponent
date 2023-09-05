#ifndef WATCHFILE_HPP
#define WATCHFILE_HPP

#include "agentUtils.hpp"
#include "service/watchservice.hpp"

class WatchController
{
private:
    string _watchDirectory;
    string _backupDirectory;
    IWatch *watcher = nullptr;

public:
    WatchController(string watchDirectory, string backupDirectory) : _watchDirectory(watchDirectory), _backupDirectory(backupDirectory), watcher(new WatchService()) {}

    void start()
    {
        if (watcher->start(_watchDirectory, _backupDirectory) == SUCCESS)
        {
            cout << "Watching files in the source directory successfully completed." << endl;
        }
    }

    ~WatchController() { delete watcher; }
};

#endif
