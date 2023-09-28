#ifndef LOG_ANALYSIS_HPP
#define LOG_ANALYSIS_HPP

#define STANDARD_TIMESTAMP_SIZE 19
#define MAX_CACHE_SIZE 5
#define RULES_DIR "/home/krishna/security/Agent/rules"

#include "service/configservice.hpp"

typedef struct p_rule p_rule;

/**
 * @brief A structure representing a custom rule for a security system.
 *
 * This structure defines a custom security rule with various attributes such as
 * an identifier, source identifiers, time-related properties, frequency counts,
 * source IP address, and a group assignment.
 */
struct p_rule
{
    int id;                  /**< The unique identifier of the rule. */
    int if_sid;              /**< The child rule id */
    int if_mid;              /**< This identifier used to match the same-category rules */
    int same_source_ip;      /**< Flag indicating if the rule considers the same source IP. */
    int same_id;             /**< Flag indicating if the rule considers the same ID. */
    int time;                /**< Time interval associated with the rule. */
    std::time_t start;       /**< Start time for the rule's validity. */
    std::time_t end;         /**< End time for the rule's validity. */
    int frequency;           /**< Frequency of rule occurrences. */
    int d_frequency;         /**< Delta frequency of rule occurrences. */
    string src_ip;           /**< Source IP address associated with the rule. */
    string group;            /**< Group assignment for the rule. */

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

    id_rule(const string& group, const int id) : group(group), id (id){}
    id_rule(){}
};

struct id_decoder
{
    string pcre2;
    string decoder;
    id_decoder(const string & pcre2, const string & decoder) : pcre2(pcre2), decoder(decoder){}
    id_decoder(){}
};

/**
 * @brief A class for handling various log analysis operations.
 *
 * The LogAnalysis class provides a set of methods and functionality for
 * performing various log analysis tasks, such as parsing, filtering, and
 * extracting insights from log data.
 */

class LogAnalysis
{
public:
    Config _configService;
    vector<p_rule> _processingRules;
    vector<int> _processedRules;
    vector<id_rule> _idRules;
    vector<string> decoder_cache;
    std::unordered_map<string, std::unordered_map<int, AConfig>> _rules;
    std::unordered_map<string, decoder> _decoder_list;

private:
    bool isValidConfig = true;
    int isRuleFound(const int ruleId);
    void addMatchedRule(const id_rule & rule, const string& log);
    string decodeGroup(log_event & logEvent);

public:
    /**
     * @brief Default constructor for the LogAnalysis class.
     *
     * Initializes an instance of the LogAnalysis class with default settings.
     */
    LogAnalysis();

    /**
     * @brief Set the configuration file paths for decoder and rule files.
     *
     * This function allows you to specify the paths to the decoder and rule files
     * for configuration. These file paths are used to initialize or update the
     * configuration members within the class.
     *
     * @param decoderPath The path to the decoder configuration file.
     * @param ruleDir The path to the directory containing rule configuration files.
     */   
    void setConfigFile(const string &decoderPath, const string &ruledDir);

    /**
     * @brief Validate a syslog entry based on its size.
     *
     * This function checks whether a syslog entry is considered valid based on its size.
     * The size of the syslog entry is compared against a specified threshold to determine
     * its validity.
     *
     * @param size The size of the syslog entry to be validated.
     *
     * @return An integer value indicating the validity of the syslog entry:
     *         - SUCCESS if the syslog entry is considered valid.
     *         - FAILED if the syslog entry is not valid due to its size.
     */
    int isValidSysLog(const size_t size); 

    /**
     * @brief Decode a log entry and extract log attributes into a log_event structure.
     *
     * This function takes a log entry string and a log format specification as input
     * and decodes the log entry, extracting relevant log attributes into a log_event
     * structure for further analysis.
     *
     * @param log The log entry string to be decoded.
     * @param format The format specification for parsing the log entry.
     *
     * @return A log_event structure containing extracted log attributes.
     */
    log_event decodeLog(const string& log, const string& format);

    void addDecoderToCache(const string & decoder);

    /**
     * @brief Format a raw syslog line into a standardized format.
     *
     * This function takes a raw syslog log entry string and a format specification as input
     * and formats the syslog entry into a standardized format according to the specified format.
     *
     * @param log The raw syslog log entry string to be formatted.
     * @param format The desired format specification for the syslog entry.
     *
     * @return A string containing the syslog entry in the standardized format.
     */
    string formatSysLog(const string& log, const string& format);

    /**
     * @brief Match a regular expression pattern against a log entry.
     *
     * This function takes a log entry string and a regular expression pattern as input
     * and checks if the log entry matches the specified pattern.
     *
     * @param log The log entry string to be matched against the pattern.
     * @param pattern The regular expression pattern to be used for matching.
     *
     * @return An integer indicating the result of the match:
     *         - 1 if the log entry matches the pattern.
     *         - 0 if the log entry does not match the pattern.
     *         - (-1) if an error occurred during the matching process.
     */
    int regexMatch(const string& log, const string& pattern, string & match);
    
    /**
     * @brief Match a PCRE2 regular expression pattern against an input string.
     *
     * This function takes an input string and a PCRE2 regular expression pattern as input
     * and uses the PCRE2 library to match the input string against the specified pattern.
     *
     * @param input The input string to be matched against the PCRE2 pattern.
     * @param pattern The PCRE2 regular expression pattern to be used for matching.
     *
     * @return An integer indicating the result of the match:
     *         - 1 if the input string matches the pattern.
     *         - 0 if the input string does not match the pattern.
     *         - (-1) if an error occurred during the matching process.
     */
    int pcreMatch(const string& input, const string& pattern, string & match, size_t & position);
    
    /*
        read the rules one by one
        checking if any rules in the memory, which is expired if it is remove it,
        checking any rules is processing state wait for logs
        Checking and do the regex matching for the logs
        Checking the read syslog size.
        Checking any rule which have child id, that might be identified before.
        Checking the timeframe or frequency is available for the rule if it is, do the given.
        And during those checkings if rule matched add the rule id to the Id'd rules.
    */
   
    /**
     * @brief Match a log event against XML-based rules.
     *
     * This function takes a log_event structure as input and performs log matching
     * against XML-based rules. It evaluates the log event against the specified rules
     * and determines if it matches any of them.
     *
     * @param logInfo The log_event structure containing log attributes to be matched.
     *
     * @return An integer indicating the result of the match:
     *         - SUCCESS if the log event matches one or more XML-based rules.
     *         - 0 if the log event does not match any XML-based rules.
     *         - (-1) if an error occurred during the matching process.
     */
    void match(log_event &logInfo);

    void match(log_event & logInfo, AConfig & ruleInfo);

    void match(log_event &logInfo, std::unordered_map<int, AConfig>& ruleSet);

    /**
     * @brief Analyze a log file by initiating log matching and managing the log matcher.
     *
     * This function takes the path to a log file as input, initiates log matching
     * and analysis for the content of the file, and manages the log matcher to process
     * log entries.
     *
     * @param file The path to the log file to be analyzed.
     *
     * @return An integer indicating the result of the log analysis:
     *         - SUCCESS if the analysis completed successfully.
     *         - FAILED if an error occurred during the analysis process.
     */
    int analyseFile(const string& file);

    /**
     * @brief Set up configuration files and start the log analysis process.
     *
     * This function allows you to set up configuration files for log analysis by
     * specifying the paths to the decoder, rules directory, and read directory. After
     * configuring the analysis parameters, it initiates the log analysis process.
     *
     * @param decoderPath The path to the decoder configuration file.
     * @param rulesDir The path to the directory containing rule configuration files.
     * @param readDir The directory where log files are located for analysis.
     *
     * @return An integer indicating the result of the log analysis:
     *         - SUCCESS if the log analysis process started successfully.
     *         - FAILED if an error occurred during setup or initiation.
     */
    int start(const string& decoderPath, const string& rulesDir, const string & readDir);

    /**
     * @brief Prepare matched data for all log matches after analysis.
     *
     * This function takes a vector of log_event structures representing matched log entries
     * and prepares the matched data for further processing or reporting. It may perform
     * post-analysis tasks on the matched data.
     *
     * @param alerts A vector of log_event structures representing matched log entries.
     *
     * @return An integer indicating the result of the post-analysis:
     *         - SUCCESS if the post-analysis completed successfully.
     *         - FAILED if an error occurred during the post-analysis process.
     */
    int postAnalysis(const vector<log_event>& alerts);

    /**
     * @brief Get an AConfig structure representing an XML-based rule.
     *
     * This function takes a group name and a rule ID as input and retrieves an AConfig
     * structure that encapsulates an XML-based rule corresponding to the specified group
     * and rule ID.
     *
     * @param group The name of the group associated with the rule.
     * @param ruleId The unique identifier of the XML-based rule.
     *
     * @return An AConfig structure representing the XML-based rule.
     */
    AConfig getRule(const string& group, const int ruleId);

    /**
     * @brief Print log and rule details for matched logs.
     *
     * This function takes an AConfig structure representing an XML-based rule and a log_event
     * structure representing a matched log entry. It prints the details of both the rule
     * and the log for reporting purpose.
     *
     * @param ruleInfo An AConfig structure representing the XML-based rule.
     * @param logInfo A log_event structure representing the matched log entry.
     *
     * @return An integer indicating the result of the printing operation:
     *         - SUCCESS if the printing completed successfully.
     *         - FAILED if an error occurred during the printing process.
     */
    int printLogDetails(const AConfig& ruleInfo, const log_event& logInfo);

    /**
     * @brief Destructor for the LogAnalysis class.
     *
     * Cleans up any resources or state associated with the LogAnalysis class.
     */
    ~LogAnalysis(){}
};

#endif