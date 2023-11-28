#pragma once

#include "common.hpp"
struct storage;

class analysis_entity {
private:
    string logfile_path;
    string decoder_path;
    string rules_dir;
    string time_pattern;
    storage storage_type;
    string rest_url;
    string rest_attribute;

public:
    // Constructor with all parameters
    analysis_entity(const string& logfile, const string& decoder, const string& rules,
                    const string& time_pattern, const storage& storage_type, const string &rest_url, const string &rest_attribute)
        : logfile_path(logfile), decoder_path(decoder), rules_dir(rules),
          time_pattern(time_pattern), storage_type(storage_type), rest_url(rest_url), rest_attribute(rest_attribute) {
    }

    // Constructor with no parameters
    analysis_entity() {
        // Initialize default values or leave members uninitialized depending on your requirements
    }

    // Getter and Setter for 'logfile_path'
    const string& getLogfilePath() const {
        return logfile_path;
    }

    void setLogfilePath(const string& logfileath) {
        logfile_path = logfileath;
    }

    // Getter and Setter for 'decoder_path'
    const string& getDecoderPath() const {
        return decoder_path;
    }

    void setDecoderPath(const string& decoder_path) {
        decoder_path = decoder_path;
    }

    // Getter and Setter for 'rules_dir'
    const string& getRulesDir() const {
        return rules_dir;
    }

    void setRulesDir(const string& rules_dir) {
        rules_dir = rules_dir;
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

    void setStorageType(const storage& storage_Type) {
        storage_type = storage_Type;
    }
     // Getter and Setter for 'rest_url'
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
