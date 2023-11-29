#pragma once

#include "common.hpp"
#include "service/loganalysis_service.hpp"

/**
 * @brief Analysis Controller
 *
 * The `analysis_controller` class is responsible for managing log analysis within the application. It serves as the
 * central component for processing and interpreting log data, providing insights, and facilitating data-driven decision-making.
 * This class plays a crucial role in extracting meaningful information from logs and enhancing application functionality.
 */
class analysis_controller
{
    private:
        log_analysis * _log_analysis = nullptr; /**< A private pointer to the log_analysis service. */
    public: 
        /**
         * @brief Construct a new Analysis Controller  object
         * This constructor initializes the `analysis_controller` and creates an instance of the `log_analysis`
         * to be used for firmware management.
         */
        analysis_controller() : _log_analysis(new log_analysis()) {} 

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

            int result = _log_analysis->start(decoderPath, rulesPath, readDir);
            cout << result << '\n';
        }
        /**
         * @brief Destructor for analysis_controller.
         *
         * The destructor performs cleanup tasks for the `analysis_controller` class, which may include
         * releasing resources and deallocating memory, such as deleting the `_log_analysis` instance.
         */
        virtual ~analysis_controller() {delete _log_analysis;}

};