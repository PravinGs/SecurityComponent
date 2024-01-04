#ifndef PROCESS_REPOSITORY_HPP
#define PROCESS_REPOSITORY_HPP

#include "model/process_model.hpp"

class process_repository
{

    public:
        int save(const vector<process_data>& logs, const sys_properties& properties, const sys_properties& availed_properties)
        {
            string path = os::get_json_write_path("process");
            Json::Value props;
            props["CpuMemory"] = properties.cpu;
            props["RamMeomry"] = properties.ram;
            props["DiskMemory"] = properties.disk;
            Json::Value availed_props;
            availed_props["CpuMemory"] = availed_properties.cpu;
            availed_props["RamMeomry"] = availed_properties.ram;
            availed_props["DiskMemory"] = availed_properties.disk;
            fstream file(path, std::ios::app);
            Json::Value jsonData;
            Json::StreamWriterBuilder writerBuilder;
            if (!file)
            {
                agent_utils::write_log("process_repository: save:" + FWRITE_FAILED + path, FAILED);
                return FAILED;
            }
            
            jsonData["DeviceTotalSpace"] = props;
            jsonData["DeviceUsedSpace"] = availed_props;
            jsonData["TimeGenerated"] = agent_utils::get_current_time();
            jsonData["Source"] = os::host_name;
            jsonData["OrgId"] = 12345;
            jsonData["ProcessObjects"] = Json::Value(Json::arrayValue);
            for (process_data data : logs)
            {
                Json::Value json_log;
                json_log["process_id"] = std::stoi(data.process_id);
                json_log["process_name"] = data.process_name;
                json_log["cpu_usage"] = std::stod(data.cpu_time);
                json_log["ram_usage"] = std::stod(data.ram_usage);
                json_log["disk_usage"] = std::stod(data.disk_usage);
                jsonData["ProcessObjects"].append(json_log);
            }

            std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
            writer->write(jsonData, &file);

            file.close();
            agent_utils::write_log("process_repository: save: " + FWRITE_SUCCESS + path, SUCCESS);
            return SUCCESS;
        }



};

#endif