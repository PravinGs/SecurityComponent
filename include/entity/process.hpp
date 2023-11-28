#pragma once
#include "common.hpp"

class process_entity {
private:
    string write_path;
    string time_pattern;
    storage storage_type;
    string rest_url;
    string rest_attribute;

public:
    // Constructor with all parameters
    process_entity(const string& write_path, const string& time_pattern, const storage& storage_type, const string &rest_url, const string &rest_attribute)
        : write_path(write_path), time_pattern(time_pattern), storage_type(storage_type), rest_url(rest_url), rest_attribute(rest_attribute) {
    }

    // Constructor with default values
    process_entity() = default;

    // Getter and Setter for 'write_path'
    const string& getWritePath() const {
        return write_path;
    }

    void setWritePath(const string& write_path) {
        write_path = write_path;
    }

    // Getter and Setter for 'time_pattern'
    const string& getTimePattern() const {
        return time_pattern;
    }

    void setTimePattern(const string& time_pattern) {
        time_pattern = time_pattern;
    }

    // Getter and Setter for 'storage_type'
    const storage& getStorageType() const {
        return storage_type;
    }

    void setStorageType(const storage& storage_type) {
        storage_type = storage_type;
    }
        const string &getRestUrl() const
    {
        return rest_url;
    }

    void setRestUrl(const string &rest_url)
    {
        rest_url = rest_url;
    }

    // Getter and Setter for 'rest_attribute'
    const string &getRestAttribute() const
    {
        return rest_attribute;
    }

    void setRestAttribute(const string &rest_attribute)
    {
        rest_attribute = rest_attribute;
    }
};
