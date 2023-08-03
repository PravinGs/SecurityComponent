#include "service/watchservice.hpp"

std::string getFormattedTime()
{
    std::time_t rawTime;
    std::time(&rawTime);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&rawTime));
    return buffer;
}

bool isDirectoryExist(const string directory)
{
    if (!(std::filesystem::exists(directory) && std::filesystem::is_directory(directory)))
    {
        AgentUtils::writeLog("No such directory : " + directory, FAILED);
        return false;
    }
    return true;
}

int WatchService::start(const string watchDirectory, const string backupDirectory)
{
    if (!isDirectoryExist(watchDirectory) || !(isDirectoryExist(backupDirectory)))
        return FAILED;

    char buffer[EVENT_BUF_LEN];
    int result = SUCCESS;
    int fd = inotify_init();
    if (fd == -1)
    {
        AgentUtils::writeLog("Failed to initialize inotify.", FAILED);
        return FAILED;
    }

    int wd = inotify_add_watch(fd, watchDirectory.c_str(), IN_CREATE | IN_MODIFY);
    if (wd == -1)
    {
        AgentUtils::writeLog("Failed to add watch to the directory.", FAILED);
        close(fd);
        return FAILED;
    }

    while (true)
    {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length <= 0)
        {
            AgentUtils::writeLog("Failed to read inotify events.", FAILED);
            result = FAILED;
            break;
        }

        int i = 0;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len > 0)
            {
                std::string fileName = event->name;
                std::string filePath = watchDirectory + "/" + fileName;
                AgentUtils::writeLog("Detected modification or creation of file: " + filePath);
                try
                {
                    std::string backupFilePath = backupDirectory + "/" + fileName + "_" + getFormattedTime();
                    std::ifstream ifs(filePath, std::ios::binary);
                    std::ofstream ofs(backupFilePath, std::ios::binary);
                    ofs << ifs.rdbuf();
                    ifs.close();
                    ofs.close();
                    break;
                }
                catch (const std::exception &e)
                {
                    string error = e.what();
                    AgentUtils::writeLog("Error during backup process: " + error, FAILED);
                    result = FAILED;
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    return result;
}

WatchService::~WatchService() {}
