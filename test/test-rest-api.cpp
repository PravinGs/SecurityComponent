// #include <iostream>
// #include <curl/curl.h>

// // Callback function to write response data
// size_t writeCallback(char* data, size_t size, size_t nmemb, std::string* response) {
//     if (response) {
//         response->append(data, size * nmemb);
//         return size * nmemb;
//     }
//     return 0;
// }

// // Callback function to retrieve the HTTP status code
// size_t headerCallback(char* buffer, size_t size, size_t nitems, long* httpCode) {
//     std::string header(buffer, size * nitems);
//     size_t pos = header.find("HTTP/1.");
//     if (pos != std::string::npos) {
//         *httpCode = std::stoi(header.substr(pos + 9, 3));
//     }
//     return size * nitems;
// }

// int main() {
//     // Initialize libcurl
//     curl_global_init(CURL_GLOBAL_DEFAULT);

//     // Create a CURL handle
//     CURL* curl = curl_easy_init();
//     if (curl) {
//         // Set the target URL
//         curl_easy_setopt(curl, CURLOPT_URL, "http://13.232.193.41/Log/AddLogs");

//         // Set the request headers
//         struct curl_slist* headers = nullptr;
//         headers = curl_slist_append(headers, "accept: */*");
//         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

//         // Set the request as a POST
//         curl_easy_setopt(curl, CURLOPT_POST, 1L);

//         // Set the request as multipart/form-data
//         struct curl_httppost* form = nullptr;
//         struct curl_httppost* last = nullptr;
//         curl_formadd(&form, &last, CURLFORM_COPYNAME, "JsonFile", CURLFORM_FILE, "/home/pravin/projects/Agent/sys.json", CURLFORM_CONTENTTYPE, "application/json", CURLFORM_END);
//         curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

//         // Perform the request
//         CURLcode res;
//         std::string response;
//         long httpCode = 0;

//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
//         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

//         curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
//         curl_easy_setopt(curl, CURLOPT_HEADERDATA, &httpCode);

//         // Enable verbose output
//         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

//         // Disable SSL verification (for testing purposes)
//         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
//         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

//         res = curl_easy_perform(curl);
//         if (res == CURLE_OK) {
//             std::cout << "Request successful." << std::endl;
//             std::cout << "HTTP Status Code: " << httpCode << std::endl;
//             std::cout << "Response: " << response << std::endl;
//         } else {
//             std::cout << "Request failed: " << curl_easy_strerror(res) << std::endl;
//         }

//         // Clean up
//         curl_slist_free_all(headers);
//         curl_easy_cleanup(curl);
//     }

//     // Cleanup libcurl
//     curl_global_cleanup();

//     return 0;
// }
