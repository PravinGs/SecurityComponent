#include "common.hpp"
#include "model/entity.hpp"

#define DEFAULT_SIGNATURE_DATABASE_PATH "/etc/scl/config/tmp/signature.txt"

class file_security
{
private:
    vector<signature_entity> read_signatures_from_file()
    {
        vector<signature_entity> entity;
        std::ifstream file(DEFAULT_SIGNATURE_DATABASE_PATH);
        string line;

        if (file.is_open())
        {
            while (std::getline(file, line))
            {
                std::istringstream iss(line);
                string file_name, signature;
                if (iss >> file_name >> signature)
                {
                    entity.push_back({file_name, signature});
                }
            }
            file.close();
        }

        return entity;
    }

    void write_signature_to_file(const signature_entity &entity)
    {
        fstream file(DEFAULT_SIGNATURE_DATABASE_PATH, std::ios::app);
        if (file.is_open())
        {
            file << entity.file_name << " " << entity.signature << '\n';
            file.close();
            agent_utils::write_log("file_security: write_signature_to_file: " + entity.file_name +" success.", DEBUG);
        }
    }

    void write_signature_to_file(const vector<signature_entity> &entities)
    {
        for (const signature_entity &entity : entities)
        {
            write_signature_to_file(entity);
        }
    }

public:
    file_security()
    {
        os::create_file(DEFAULT_SIGNATURE_DATABASE_PATH);
    }

    int sign_and_store_signature(const string &file, const string &sign_key)
    {
        string signature_data = os::sign(file, sign_key);

        if (signature_data.empty())
        {
            agent_utils::write_log("file_security: sign_and_store_signature: signed data is empty", FAILED);
            return FAILED;
        }

        signature_entity entity(file, signature_data);

        write_signature_to_file(entity);

        return SUCCESS;
    }

    bool verify_signature(const string &file_name, const string &sign_key)
    {
        vector<signature_entity> entities = read_signatures_from_file();
        auto it = std::remove_if(entities.begin(), entities.end(), [&file_name, &sign_key](const signature_entity &entity)
                                 {
            if (entity.file_name == file_name && os::sign(file_name, sign_key) == entity.signature)
            {
                agent_utils::write_log("file_security: verify_and_remove_signature: signature verification successfull. removing entry.", DEBUG);
                return true;
            }
            return false; });

        entities.erase(it, entities.end());
        write_signature_to_file(entities);
        return it != entities.end();
    }
};