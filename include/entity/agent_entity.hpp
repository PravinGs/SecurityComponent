#pragma once
#include "common.hpp"
#include "log_analysis.hpp"
#include "log_collector.hpp"
#include "process.hpp"
#include "ids.hpp"
#include "rapi.hpp"

class agent_entity {
private:
    log_entity syslog;
    log_entity applog;
    analysis_entity log_analysis;
    process_entity process;
    ids_entity ids;
    api_entity rest_service;

public:
    // Constructor with all parameters
    agent_entity(log_entity& syslog, log_entity& applog, analysis_entity& log_analysis,
                 process_entity& process, ids_entity& ids,
                 api_entity& rest_service)
        : syslog(syslog), applog(applog), log_analysis(log_analysis), process(process), ids(ids),
          rest_service(rest_service) {
    }

    // Constructor with default values
    agent_entity()
    {
        
    }

    // Getter and Setter for 'syslog'
    const log_entity& getSysLog() const {
        return syslog;
    }

    void setSysLog(log_entity& log) {
        this->syslog = log;
    }

    // Getter and Setter for 'applog'
    const log_entity& getAppLog() const {
        return applog;
    }

    void setAppLog(log_entity& log) {
        this->applog = log;
    }

    // Getter and Setter for 'log_analysis'
    const analysis_entity& getLogAnalysis() const {
        return log_analysis;
    }

    void setLogAnalysis(analysis_entity& log_analysis) {
        this->log_analysis = log_analysis;
    }

    // Getter and Setter for 'process'
    const process_entity& getProcess() const {
        return process;
    }

    void setProcess(process_entity& process) {
        this->process = process;
    }

    // Getter and Setter for 'ids'
    const ids_entity& getIds() const {
        return ids;
    }

    void setIds(ids_entity& ids) {
        this->ids = ids;
    }

    // Getter and Setter for 'rest_service'
    const api_entity& getRestService() const {
        return rest_service;
    }

    void setRestService(api_entity& rest_service) {
        this->rest_service = rest_service;
    }
};
