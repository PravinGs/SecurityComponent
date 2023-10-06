#pragma once

#include "agentUtils.hpp"
#include "service/loganalysis_service.hpp"

/**
 * @brief Analysis Controller
 *
 * The `AnalysisController` class is responsible for managing log analysis within the application. It serves as the
 * central component for processing and interpreting log data, providing insights, and facilitating data-driven decision-making.
 * This class plays a crucial role in extracting meaningful information from logs and enhancing application functionality.
 */
class AnalysisController
{
    private:
        LogAnalysis * _logAnalysis = nullptr; /**< A private pointer to the LogAnalysis service. */
    public: 
        /**
         * @brief Construct a new Analysis Controller  object
         * This constructor initializes the `AnalysisController` and creates an instance of the `LogAnalysis`
         * to be used for firmware management.
         */
        AnalysisController() : _logAnalysis(new LogAnalysis()) {} 

        /**
         * @brief Start Log Analysis Operation
         *
         * The `start` function is responsible for initiating the log analysis operation. It validates the information provided
         * in the `table` parameter to ensure it meets the required criteria for analysis. Once validation is successful, it
         * triggers the log analysis process.
         *
         * @param[in] table A map containing configuration data and log information for analysis.
         *                  The map should be structured to include necessary settings and log data.
         */
        void start(map<string,  map<string, string>>& table)
        {
            string decoderPath = table["log_analysis"]["decoder_path"];
            string rulesPath = table["log_analysis"]["rules_path"];
            string readDir = table["log_analysis"]["read_dir"];

            if (decoderPath.empty() || rulesPath.empty()) return;

            int result = _logAnalysis->start(decoderPath, rulesPath, readDir);
        }
        /**
         * @brief Destructor for AnalysisController.
         *
         * The destructor performs cleanup tasks for the `AnalysisController` class, which may include
         * releasing resources and deallocating memory, such as deleting the `_logAnalysis` instance.
         */
        virtual ~AnalysisController() {delete _logAnalysis;}

};