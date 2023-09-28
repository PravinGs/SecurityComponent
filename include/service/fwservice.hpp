#ifndef FWSERVICE_HPP
#define FWSERVICE_HPP

#include "agentUtils.hpp"

#define MAX_RETRY_COUNT 3
#define TIME_OUT_ERROR 12

typedef struct download_props download_props;

/**
 * @brief Download Properties Structure
 *
 * The `download_props` struct defines properties for downloading firmware or files. It contains information such as
 * file size, download speed limits, timeout, retry count, file paths, and URLs.
 *
 * @note The constructor initializes some members to default values.
 *
 * @var size The current size of the file to be downloaded (in bytes).
 * @var maxSpeed The maximum download speed limit (in bytes per second).
 * @var minSpeed The minimum download speed limit (in bytes per second).
 * @var timeout The download timeout in seconds.
 * @var retry The maximum number of download retries.
 * @var writePath The path where the downloaded file will be saved.
 * @var url The URL from which the file is to be downloaded.
 * @var fileName The name of the downloaded file.
 * @var downloadPath The complete path to the downloaded file.
 */
struct download_props
{
   long size;
   int maxSpeed;
   int minSpeed;
   int timeout;
   int retry;
   string writePath;
   string rootDir;
   string url;
   string fileName;
   string downloadPath;
   
   /**
    * @brief Default Constructor
    *
    * The default constructor initializes some members to default values:
    * - `size` to 0
    * - `maxSpeed` to 0
    * - `minSpeed` to 0
    * - `timeout` to 0
    * - `retry` to a predefined maximum retry count (MAX_RETRY_COUNT)
    */
   download_props() : size(0L), maxSpeed(0), minSpeed(0), timeout(0), retry(MAX_RETRY_COUNT) {}
};

struct sftp_data
{
    string url;
    string username;
    string password;
};
/**
 * @brief Interface for Firmware Download Service
 *
 * The `IFService` interface defines a set of functions for starting a firmware download service. Classes that implement
 * this interface are responsible for downloading firmware based on the provided configuration table.
 */
class IFService
{
public:
    /**
     * @brief Start Firmware Download
     *
     * The `start` function is a pure virtual method that starts the firmware download service based on a configuration
     * table. Derived classes must implement this method to provide specific functionality for firmware downloads.
     *
     * @param[in] configTable A map containing configuration data for firmware downloads.
     *                       The map should be structured to specify download parameters.
     * @return An integer result code indicating the success or failure of the firmware download operation.
     */
    virtual int start(map<string, map<string, string>>& configTable) = 0;

    /**
     * @brief Destructor
     *
     * The virtual destructor for the `IFService` interface is provided to ensure proper cleanup when derived classes
     * are destroyed.
     */
    virtual ~IFService() {}
};

/**
 * @brief Firmware Download Service Implementation
 *
 * The `FService` class is an implementation of the `IFService` interface. It provides functionality to start a firmware
 * download service based on the provided configuration table.
 */
class FService : public IFService
{
private:
    CURL *curl = nullptr;
    long fileSize = 0L;
    download_props dProperties;
    string username, password;

private:
    
    /**
     * @brief Get File Size
     *
     * The `readFileSize` function is used to retrieve the size of a file pointed to by the provided `FILE` pointer.
     *
     * @param[in] file A pointer to the open file for which the size is to be determined.
     * @return The size of the file in bytes, or -1 if an error occurred while obtaining the file size.
     */
    long readFileSize(FILE *file);
        
    /**
     * @brief Create Download Properties from Configuration Table
     *
     * The `createDProps` function is used to construct a `download_props` structure by extracting firmware section details
     * from the provided configuration table. It populates the `download_props` structure with values specified in the
     * configuration table.
     *
     * @param[in] table A map containing configuration data, including firmware section details.
     * @return An integer result code:
     *         - SUCCESS: The `download_props` structure was successfully created and populated.
     *         - FAILED: The operation encountered errors and failed to create or populate the structure.
     */
    int createDProps(map<string, map<string, string>>& table);

    /**
     * @brief Extract File Name from URL
     *
     * The `extractFileName` function is used to extract the file name from a given URL. It parses the URL and retrieves the
     * name of the file that is going to be downloaded.
     *
     * @param[in] url The URL from which the file name needs to be extracted.
     * @return A string representing the extracted file name.
     */
    string extractFileName(const string &url);
    
    /**
     * @brief Download File via HTTPS Protocol
     *
     * The `download` function without parameters is used to initiate a file download using the HTTPS protocol. It downloads
     * the specified file from a remote server over a secure connection.
     *
     * @return An integer result code indicating the success or failure of the download operation.
     */
    int download();

    /**
     * @brief Download File via SFTP Protocol with Credentials
     *
     * The `download` function with parameters `username` and `password` is used to initiate a file download using the SFTP
     * protocol. It downloads the specified file from a remote server over a secure connection with the provided
     * authentication credentials.
     *
     * @param[in] username The username or account name for SFTP authentication.
     * @param[in] password The password or authentication token for SFTP authentication.
     * @return An integer result code indicating the success or failure of the SFTP download operation.
     */
    int download(const string& username, const string& password);
    
public:

    /**
     * @brief Constructor for FService
     *
     * The constructor for the `FService` class initializes an instance of the class. It sets up the necessary resources
     * and configurations for the firmware download service.
     */
    FService(); 

    /**
     * @brief Start Firmware Download
     *
     * The `start` function is implemented to start the firmware download service based on the provided configuration
     * table. It handles the firmware download process according to the specified parameters.
     *
     * @param[in] configTable A map containing configuration data for firmware downloads.
     *                       The map should be structured to specify download parameters.
     * @return An integer result code indicating the success or failure of the firmware download operation.
     */
    int start(map<string, map<string, string>>& configTable);
    
    /**
     * @brief Destructor for FService
     *
     * The destructor for the `FService` class performs cleanup and resource release when an instance of the class is
     * destroyed. It ensures that any allocated resources are properly released.
     */
    ~FService();
};

#endif