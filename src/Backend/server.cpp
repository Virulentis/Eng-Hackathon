#include "crow.h"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

// Write callback: stores response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string searchCanvas(std::string token){
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string course_id = "92902";

    std::string base_url = "https://canvas.sfu.ca/api/v1/users/self";
    std::string url = base_url + "/courses/" + course_id + "/assignments";

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return "ERROR: curl init failed";
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "eng-hackathon/1.0 (curl)");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    // Clean up headers and curl before returning
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Network/transfer errors
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        return "ERROR: " + std::string(curl_easy_strerror(res));
    }

    // Treat any non-2xx HTTP status as an application-level error
    if (http_code < 200 || http_code >= 300) {
        std::cerr << "HTTP error: " << http_code << "\n";
        std::cerr << "Response body:\n" << readBuffer << "\n";
        return "ERROR: HTTP " + std::to_string(http_code);
    }

    return readBuffer;
}


int main() {
    crow::SimpleApp app;
    
    // Enable CORS for all routes
    CROW_ROUTE(app, "/api/hello")
    ([](){
        crow::response res("Request Sent!");
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        return res;
    });
    
    CROW_ROUTE(app, "/api/data").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req){
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 200;
            return res;
        }
        
        std::string received = req.body;
        std::cout << "Received: " << received << std::endl;
        res.body = "Server got: " + received;
        return res;
    });
    
    CROW_ROUTE(app, "/api/find_classes").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req){
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        
        // Handle OPTIONS first, before making any API calls
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 200;
            return res;
        }
        
        std::string received = req.body;
        std::cout << "Received token: " << received << std::endl;
        
        // Now make the API call
        std::string result = searchCanvas(received);
        std::cout << "Canvas API result: " << result.substr(0, 100) << "...\n";
        
        res.body = result;
        return res;
    });

    std::cout << "Server running on http://localhost:8080" << std::endl;
    app.port(8080).multithreaded().run();
}