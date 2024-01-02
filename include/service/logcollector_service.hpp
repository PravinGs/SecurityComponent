#ifndef log_service_HPP
#define log_service_HPP
#pragma once

#include "common.hpp"
#include "service/config_service.hpp"
#include "udp.hpp"
#include "model/entity.hpp"
#include "repository/logcollector_repository.hpp"

class ILog
{
public:
    virtual int get_syslog(log_entity &entity) = 0;

    virtual int get_applog(log_entity &entity) = 0;

    virtual ~ILog() {}
};

class log_service : public ILog
{
private:
    Config config; /**< A private instance of IniConfig for configuration management. */
    logcollector_repository db;
    map<string, int> log_parser_level{{"none", 0}, {"trace", 1}, {"debug", 2}, {"warning", 3}, {"error", 4}, {"critical", 5}}; /**< A private constant map<string, int> for system log name. */

private:
    int read_syslog_file(log_entity &entity, vector<string> &logs);

    int read_applog_file(log_entity &entity, vector<string> &logs);

    bool parse_log_category(string &line, const vector<string> &log_levels);

    int read_dpkg_logfile(log_entity &entity, vector<string> &logs);

    int read_remote_syslog(UdpQueue &queue, vector<string> &logs);

public:
    log_service() = default;

    int get_syslog(log_entity &entity);

    int get_applog(log_entity &entity);

    ~log_service();
};

#endif