#pragma once

#include "common.hpp"


class ids_entity {
private:
    string source_dir;
    string search_dir;
    storage storage_type;
    string rest_url;
    string rest_attribute;

public:
    // Constructor with all parameters
    ids_entity(const string& source_dir, const string& search_dir, const storage& storage_type,  const string &rest_url, const string &rest_attribute)
        : source_dir(source_dir), search_dir(search_dir), storage_type(storage_type), rest_url(rest_url), rest_attribute(rest_attribute)  {
    }

    // Constructor with default values
    ids_entity() = default;

    // Getter and Setter for 'source_dir'
    const string& getSourceDir() const {
        return source_dir;
    }

    void setSourceDir(const string& source_dir) {
        this->source_dir = source_dir;
    }

    // Getter and Setter for 'search_dir'
    const string& getSearchDir() const {
        return search_dir;
    }

    void setSearchDir(const string& search_dir) {
        this->search_dir = search_dir;
    }

    // Getter and Setter for 'storage_type'
    const storage& getStorageType() const {
        return storage_type;
    }

    void setStorageType(const storage& storage_type) {
        this->storage_type = storage_type;
    }

        const string &getRestUrl() const
    {
        return rest_url;
    }

    void setRestUrl(const string &rest_url)
    {
        this->rest_url = rest_url;
    }

    // Getter and Setter for 'rest_attribute'
    const string &getRestAttribute() const
    {
        return rest_attribute;
    }

    void setRestAttribute(const string &rest_attribute)
    {
        this->rest_attribute = rest_attribute;
    }
};