#ifndef FWSERVICE_HPP
#define FWSERVICE_HPP

#include "model/patch_model.hpp"

class Ipatch_service
{
public:
    virtual int start(map<string, map<string, string>> &config_table) = 0;

    virtual ~Ipatch_service() {}
};

class patch_service : public Ipatch_service
{
private:
    CURL *curl = nullptr;
    long file_size = 0L;
    download_props d_properties;
    string username, password;

private:
    long readFileSize(FILE *file);

    int create_download_property(map<string, map<string, string>> &table);

    string extract_ile_name(const string &url);

    int download();

    int download(const string &username, const string &password);

public:
    patch_service();

    int start(map<string, map<string, string>> &config_table);

    ~patch_service();
};

#endif