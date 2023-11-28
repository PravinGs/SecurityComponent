#pragma once
#include "common.hpp"

class log_entity
{
private:
    string type;
    string read_path;
    string delimeter;
    string write_path;
    vector<string> commands;
    string time_pattern;
    storage storage_type;
    string rest_url;
    string rest_attribute;
    

public:
    log_entity()
    {
        
    }
    log_entity(const string &type, const string &read_path, const string& delimeter,
                                 const string &write_path, vector<string> &commands, const string &time_pattern,
                                 const storage &storage_type, const string &rest_url, const string &rest_attribute) : type(type), read_path(read_path), delimeter(delimeter), write_path(write_path), commands(commands), time_pattern(time_pattern), storage_type(storage_type), rest_url(rest_url), rest_attribute(rest_attribute)
    {
    }


    ~log_entity()
    {
    }

    // Getter and Setter for 'type'
    const string &getType() const
    {
        return type;
    }

    void setType(const string &new_type)
    {
        this->type = new_type;
    }

    // Getter and Setter for 'read_path'
    const string &getReadPath() const
    {
        return read_path;
    }

    void setReadPath(const string &read_path)
    {
        this->read_path = read_path;
    }

    // Getter and Setter for 'delimeter'
    const string& getDelimeter() const
    {
        return delimeter;
    }

    void setDelimeter(const string& delimeter)
    {
       this->delimeter = delimeter;
    }

    // Getter and Setter for 'write_path'
    const string &getWritePath() const
    {
        return write_path;
    }

    void setWritePath(const string &write_path)
    {
        this->write_path = write_path;
    }

    // Getter and Setter for 'commands'
    const std::vector<string> &getCommands() const
    {
        return commands;
    }

    void setCommands(const std::vector<string> &commands)
    {
        this->commands = commands;
    }

    // Getter and Setter for 'time_pattern'
    const string &getTimePattern() const
    {
        return time_pattern;
    }

    void setTimePattern(const string &time_pattern)
    {
        this->time_pattern = time_pattern;
    }

    // Getter and Setter for 'storage_type'
    const storage &getStorageType() const
    {
        return storage_type;
    }

    void setStorageType(const storage &storage_type)
    {
        this->storage_type = storage_type;
    }

    // Getter and Setter for 'rest_url'
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
