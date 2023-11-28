#pragma once
#include "common.hpp"
#include "log_analysis.hpp"
#include "log_collector.hpp"
#include "process.hpp"
#include "ids.hpp"
#include "rapi.hpp"

class agent_entity {
private:
    log_entity log;
    analysis_entity log_analysis;
    process_entity process;
    ids_entity ids;
    api_entity rest_service;

public:
    // Constructor with all parameters
    agent_entity(const log_entity& log, const analysis_entity& log_analysis,
                 const process_entity& process, const ids_entity& ids,
                 const api_entity& rest_service)
        : log(log), log_analysis(log_analysis), process(process), ids(ids),
          rest_service(rest_service) {
    }

    // Constructor with default values
    agent_entity() = default;

    // Getter and Setter for 'log'
    const log_entity& getLog() const {
        return log;
    }

    void setLog(const log_entity& log) {
        log = log;
    }

    // Getter and Setter for 'log_analysis'
    const analysis_entity& getLogAnalysis() const {
        return log_analysis;
    }

    void setLogAnalysis(const analysis_entity& log_analysis) {
        log_analysis = log_analysis;
    }

    // Getter and Setter for 'process'
    const process_entity& getProcess() const {
        return process;
    }

    void setProcess(const process_entity& process) {
        process = process;
    }

    // Getter and Setter for 'ids'
    const ids_entity& getIds() const {
        return ids;
    }

    void setIds(const ids_entity& ids) {
        ids = ids;
    }

    // Getter and Setter for 'rest_service'
    const api_entity& getRestService() const {
        return rest_service;
    }

    void setRestService(const api_entity& rest_service) {
        rest_service = rest_service;
    }
};
