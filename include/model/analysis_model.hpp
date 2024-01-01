#pragma once

#include "common.hpp"

#define STANDARD_TIMESTAMP_SIZE 19
#define MAX_CACHE_SIZE 5
#define IDS_DEFAULT_XML_RULES_PATH "/home/krishna/security/Agent/rules"
#define IDS_DEFAULT_DECODER_RULES_PATH "/etc/scl/decoder/decoder.xml"

typedef struct p_rule p_rule;
typedef struct aconfig aconfig;
typedef struct log_event log_event;
typedef struct decoder decoder;

struct decoder
{
    string decode;
    string parent;
    string program_name_pcre2;
    string pcre2;
    string order;
    string prematch_pcre2;
    string fts;
    string prematch_offset;
    string pcre2_offset;

    void update(const decoder &other)
    {
        if (decode.empty())
            decode = other.decode;
        if (parent.empty())
            parent = other.parent;
        if (program_name_pcre2.empty())
            program_name_pcre2 = other.program_name_pcre2;
        if (pcre2.empty())
            pcre2 = other.pcre2;
        if (order.empty())
            order = other.order;
        if (prematch_pcre2.empty())
            prematch_pcre2 = other.prematch_pcre2;
        if (fts.empty())
            fts = other.fts;
        if (prematch_offset.empty())
            prematch_offset = other.prematch_offset;
        if (pcre2_offset.empty())
            pcre2_offset = other.pcre2_offset;
    }
};

struct log_event
{
    size_t size;      /**< The size of the log event. */
    string log;       /**< The original log entry string. */
    string format;    /**< The log format (e.g., syslog, auth, dpkg, netstat, port). */
    string timestamp; /**< The timestamp associated with the log event. */
    string program;   /**< The program or source of the log event. */
    string user;      /**< The user associated with the log event. */
    string message;
    string src_ip;  /**< The source IP address in the log event. */
    string dest_ip; /**< The destination IP address in the log event. */
    string proto;   /**< The protocol used in the log event. */
    int is_matched; /**< A flag indicating if the log event matched a rule (0 or 1). */
    string group;   /**< The group associated with the matched rule. */
    string decoded;
    int rule_id; /**< The ID of the rule that matched the log event. */

    /**
     * @brief Default constructor for the log_event structure.
     *
     * Initializes an instance of the log_event structure with default values
     * for its attributes.
     */
    log_event() : size(0L), is_matched(0), rule_id(0) {}
};

struct aconfig
{
    int id;
    int level;
    int if_sid;
    int if_matched_id;
    int same_source_ip;
    int frequency;
    int timeframe;
    int same_id;
    int noalert;
    int different_url;
    int max_log_size;

    string group;
    string decoded_as;
    string description;
    vector<string> pcre2;
    string info;
    string type;
    string status_pcre2;
    string extra_data_pcre2;
    string options;
    string user_pcre2;
    string if_group;
    string if_matched_group;
    string id_pcre2;
    string action;
    string categories;
    string check_if_ignored;
    string ignore;
    string regex;
    string script;
    string program_name_pcre2;
    string weekday;
    string time;
    string url_pcre2;
    string if_fts;
    string hostname_pcre2;
    string match;
    string compiled_rule;

    aconfig() : id(0), level(0), if_sid(0), if_matched_id(0), same_source_ip(0), frequency(0),
                timeframe(0), same_id(0), noalert(0), different_url(0), max_log_size(0)
    {
        group = "";
        decoded_as = "";
        pcre2.clear();
        description = "";
        info = "";
        type = "";
        status_pcre2 = "";
        program_name_pcre2 = "";
        regex = "";
        user_pcre2 = "";
        compiled_rule = "";
        match = "";
        hostname_pcre2 = "";
        if_fts = "";
        time = "";
        weekday = "";
        script = "";
        check_if_ignored = "";
        categories = "";
        if_group = "";
        action = "";
        id_pcre2 = "";
        if_matched_group = "";
        options = "";
        extra_data_pcre2 = "";
    }
};

struct p_rule
{
    int id;             /**< The unique identifier of the rule. */
    int if_sid;         /**< The child rule id */
    int if_mid;         /**< This identifier used to match the same-category rules */
    int same_source_ip; /**< Flag indicating if the rule considers the same source IP. */
    int same_id;        /**< Flag indicating if the rule considers the same ID. */
    int time;           /**< Time interval associated with the rule. */
    std::time_t start;  /**< Start time for the rule's validity. */
    std::time_t end;    /**< End time for the rule's validity. */
    int frequency;      /**< Frequency of rule occurrences. */
    int d_frequency;    /**< Delta frequency of rule occurrences. */
    string src_ip;      /**< Source IP address associated with the rule. */
    string group;       /**< Group assignment for the rule. */

    /**
     * @brief Default constructor for the p_rule structure.
     *
     * Initializes the p_rule structure with default values for its attributes.
     */
    p_rule() : id(0), if_sid(0), if_mid(0), same_source_ip(0),
               same_id(0), time(0), frequency(0), d_frequency(0) {}
};

struct id_rule
{
    string group;
    int id;

    id_rule(const string &group, const int id) : group(group), id(id) {}
    id_rule() {}
};

struct id_decoder
{
    string pcre2;
    string decoder;
    id_decoder(const string &pcre2, const string &decoder) : pcre2(pcre2), decoder(decoder) {}
    id_decoder() {}
};
