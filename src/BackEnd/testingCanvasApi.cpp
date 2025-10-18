#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <string>

// Write callback: stores response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}


std::string searchCanvas(std::string tail_url){
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string base_url = "https://canvas.sfu.ca/api/v1/users/self";
    std::string token = "7h3KhXftyMJL9c6nezhFk7kuZCQcL4Uk6VKa3WAf8QtmCcGQRcvUw2ePA8Jy9CUT";
    std::string url = base_url + tail_url;

    curl = curl_easy_init();
    if (!curl) return "";

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

    // Always cleanup libcurl resources before returning
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        return "";
    }

    if (http_code < 200 || http_code >= 300) {
        std::cerr << "HTTP error: " << http_code << "\n";
        std::cerr << "Response body (may contain HTML/login page):\n" << readBuffer << "\n";
        return "";
    }

    return readBuffer;
}

std::vector<std::string> getCourseIDsFromCanvas(){
    std::string readBuffer = searchCanvas("/courses");
    std::vector<std::string> ids;
    if (readBuffer.empty()) {
        std::cerr << "Failed to fetch courses or empty response.\n";
        return ids;
    }

    // std::cout << readBuffer << std::endl; // DEBUG

    try {
        auto j = nlohmann::json::parse(readBuffer);
        if (j.is_array()) {
            for (const auto& item : j) {
                std::string name = item.value("name", "");
                bool access_restricted = item.value("access_restricted_by_date", false);
                int idnum = item.value("id", 0); // read id as number to avoid type error
                if (!access_restricted) {
                    if (idnum != 0) ids.push_back(std::to_string(idnum));
                }
                std::cout << "id=" << idnum << " name=" << name << "\n";
            }
        }
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }

    return ids;
}

void searchCanvasAssignments(){
    std::vector<std::string> course_ids = getCourseIDsFromCanvas();
    for(int i=0;i<course_ids.size();i++){
        std::string tail_url = "/courses/" + course_ids.at(i) + "/assignments";
        std::string readBuffer = searchCanvas(tail_url);
        // std::cout << readBuffer << std::endl; //DEBUG
        std::cout << "\n\n\n";
        
        auto j = nlohmann::json::parse(readBuffer);

        // If the API returns an array (e.g. assignments), iterate:
        if (j.is_array()) {
            for (const auto& item : j) {
                std::string name = item.value("name", "");
                std::string created_at = item.value("created_at","");
                std::string due_at = item.value("created_at","");
                
                int id = item.value("id", 0);
                std::cout << "id=" << id << " name=" << name << " created_at=" << created_at << " due_at=" << due_at << "\n";
            }
        }
        std::cout << i + "--------------" << "\n\n";
    }
}

int main() {
    // std::vector<std::string> ids = getCourseIDsFromCanvas();
    searchCanvasAssignments();

    return 0;
}
