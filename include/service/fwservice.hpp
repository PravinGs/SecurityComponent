#ifndef FWSERVICE_HPP
#define FWSERVICE_HPP

#include "agentUtils.hpp"

class IFService
{
public:
    virtual int download(map<string, map<string, string>> configTable) = 0;
    virtual ~IFService() {}
};

class FService : public IFService
{
private:
    CURL *curl = nullptr;
    long fileSize = 0L;

private:
    void updateCurl(CURL *curl);
    long readFileSize(FILE *file);
    void setFileSize(long size);

public:
    FService() {}
    int download(map<string, map<string, string>> configTable);
    ~FService() {}
};

#endif