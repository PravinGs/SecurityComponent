#ifndef FWSERVICE_HPP
#define FWSERVICE_HPP

#include "model/patch_model.hpp"

class Ipatch_service
{
public:
    virtual int start(patch_entity& entity) = 0;

    virtual ~Ipatch_service() {}
};

class patch_service : public Ipatch_service
{
private:
    CURL *curl = nullptr;

private:
    long seek_file_size(FILE *file);

    int download(patch_entity& entity);

public:
    patch_service();

    int start(patch_entity& entity);

    ~patch_service();
};

#endif