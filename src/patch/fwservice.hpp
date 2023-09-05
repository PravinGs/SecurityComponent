#ifndef FWSERVICE_HPP
#define FWSERVICE_HPP

#include "utils/agentUtils.hpp"

#define MAX_RETRY_COUNT 3
#define TIME_OUT_ERROR 12

typedef struct D_PROP D_PROP;

struct D_PROP
{
   long size;
   int maxSpeed;
   int minSpeed;
   int timeout;
   int retry;
   string writePath;
   string url;
   string fileName;
   string downloadPath;
   
   D_PROP() : size(0L), maxSpeed(0), minSpeed(0), timeout(0), retry(MAX_RETRY_COUNT) {}

};

class IFService
{
public:
    virtual int start(map<string, map<string, string>> configTable) = 0;
    virtual ~IFService() {}
};

class FService : public IFService
{
private:
    CURL *curl = nullptr;
    long fileSize = 0L;
    D_PROP dProperties;
    string username, password;

private:
    long readFileSize(FILE *file);
    void setTerminalSetting(bool terminalMode);
    int createDProps(const map<string, map<string, string>> table);
    string extractFileName(const string &url);
    int download();
    int download(const string username, const string password);
    
public:
    FService();
    int start(const map<string, map<string, string>> configTable);
    ~FService();
};

#endif