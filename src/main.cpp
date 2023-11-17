#include "controller/main_controller.hpp"
#include "common.hpp"
#ifndef NO_DEBUG
#define NO_DEBUG
#endif

void handle_message(const string &json_string)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream iss(json_string);
    std::string errs;

    // Parse the JSON string
    Json::parseFromStream(builder, iss, &root, &errs);

    if (!errs.empty())
    {
        std::cerr << "Error: Failed to parse JSON: " << errs << "\n";
        return;
    }

    // Access the array under "ApparmorStatus"
    const Json::Value &apparmorStatusArray = root["ApparmorStatus"];

    // Ensure it's an array
    if (!apparmorStatusArray.isArray())
    {
        std::cerr << "Error: ApparmorStatus is not an array\n";
        return;
    }

    // Print the elements of the array
    for (const auto &status : apparmorStatusArray)
    {
        std::cout << status.asString() << "\n";
    }

    return;
}

int main()
{
    openlog("agent.service", LOG_INFO | LOG_CONS, LOG_USER);
    agent_utils::syslog_enabled = true;
    agent_utils::setup_logger();
    if (!agent_utils::syslog_enabled)
    {
        agent_utils::logfp.open(LOG_PATH, std::ios::app);
    }

    main_controller controller(AGENT_CONFIG_DIR);
    controller.start();
    if (agent_utils::logfp.is_open())
    {
        agent_utils::logfp.close();
    }
    closelog();
    return 0;
}