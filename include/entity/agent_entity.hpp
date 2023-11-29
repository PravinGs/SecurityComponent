#pragma once
#include "common.hpp"
#include "analysis_entity.hpp"
#include "entity/log_entity.hpp"
#include "entity/ids_entity.hpp"
#include "entity/process_entity.hpp"
#include "entity/api_entity.hpp"

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
    const log_entity& getSysLogEntity() const {
        return syslog;
    }

    void setSysLog(const log_entity& log) {
        this->syslog = log;
    }

    // Getter and Setter for 'applog'
    const log_entity& getAppLogEntity() const {
        return applog;
    }

    void setAppLog(const log_entity& log) {
        this->applog = log;
    }

    // Getter and Setter for 'log_analysis'
    const analysis_entity& getAnalysisEntity() const {
        return log_analysis;
    }

    void setLogAnalysis(const analysis_entity& log_analysis) {
        this->log_analysis = log_analysis;
    }

    // Getter and Setter for 'process'
    const process_entity& getProcessEntity() const {
        return process;
    }

    void setProcess(const process_entity& process) {
        this->process = process;
    }

    // Getter and Setter for 'ids'
    const ids_entity& getIdsEntity() const {
        return ids;
    }

    void setIds(const ids_entity& ids) {
        this->ids = ids;
    }

    // Getter and Setter for 'rest_service'
    const api_entity& getRestService() const {
        return rest_service;
    }

    void setRestService(const api_entity& rest_service) {
        this->rest_service = rest_service;
    }
};
