#pragma once

#include "agentUtils.hpp"
#include "service/loganalysis.hpp"

class AnalysisController
{
    private:
        LogAnalysis * logAnalysis = nullptr;
    public: 
        AnalysisController() : logAnalysis(new LogAnalysis())
        {}

        void start(map<string,  map<string, string>>& table)
        {
            string decoderPath = table["log_analysis"]["decoder_path"];
            string rulesPath = table["log_analysis"]["rules_path"];
            string readDir = table["log_analysis"]["read_dir"];

            if (decoderPath.empty() || rulesPath.empty()) return;

            int result = logAnalysis->start(decoderPath, rulesPath, readDir);
        }

        virtual ~AnalysisController() {delete logAnalysis;}

};