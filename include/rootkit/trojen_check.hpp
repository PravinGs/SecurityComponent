#ifndef TROJEN_CHECK_CPP
#define TROJEN_CHECK_CPP
#pragma once


#include "common.hpp"

class trojan_check
{
private:
    vector<string> rootFolders = {"bin", "sbin", "usr/bin", "usr/sbin"};
    const string baseDirectory = "/";
    int detected;
    int total;

    bool patternMatching(const string file, const string pattern)
    {
        std::ifstream fp(file, std::ios::binary);

        if (!fp.is_open())
        {
            agent_utils::write_log("trojan_check: patternMatching: " + FILE_ERROR + file, FAILED);
            return false;
        }
        char buffer[OS_SIZE_1024];
        string content;
        while (fp.read(buffer, OS_SIZE_1024))
        {
            content.append(buffer, fp.gcount());
        }
        if (fp.bad())
        {
            agent_utils::write_log("trojen_check: patternMatching: " + FREAD_FAILED + file, FAILED);
            return false;
        }
        std::regex regex(pattern);
        return std::regex_search(content, regex);
    }

public:

    int check(const string filePath, vector<string> & reports)
    {
        fstream fp(filePath, std::ios::in);
        string line;
        if (!fp)
        {
            agent_utils::write_log("trojen_check: check: " + FILE_ERROR + filePath, FAILED);
            return FAILED;
        }
        detected = 0;
        total = 0;

        while (std::getline(fp, line))
        {
            if (line.empty() || line[0] == '#')
                continue;
            int start = 0;
            int mid = (int)line.find_first_of('!');
            int end = (int)line.find_last_of('!');
            string searchBinary = agent_utils::trim(line.substr(start, mid));
            string pattern = agent_utils::trim(line.substr(mid + 1, (end - mid - 1)));
            total++;
            for (string folder : rootFolders)
            {
                string path = baseDirectory + folder + "/" + searchBinary;
                if (!std::filesystem::exists(path))
                    continue;
                if (patternMatching(path, pattern))
                {
                    detected = 1;
                    reports.push_back(path+","+pattern);
                    string errorMessage = "Trojaned version of file " + path + " detected. Signature used: " + pattern;
                    agent_utils::write_log("trojen_check: check: " + errorMessage, CRITICAL);
                }
            }
        }

        if (detected == 0)
        {
            string errorMessage = "No binaries with any trojan detected. Analyzed " + std::to_string(total) + " files.";
            agent_utils::write_log("trojen_check: check: " + errorMessage);
        }
        return SUCCESS;
    }
};

#endif