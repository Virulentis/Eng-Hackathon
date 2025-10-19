#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

// Write callback: stores response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string searchCanvas(string token){
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string course_id = "92902";

    std::string base_url = "https://canvas.sfu.ca/api/v1/users/self";
    std::string account_id = "204";
    std::string token = "7h3KhXftyMJL9c6nezhFk7kuZCQcL4Uk6VKa3WAf8QtmCcGQRcvUw2ePA8Jy9CUT";
    std::string url = base_url + "/courses/" + course_id + "/assignments";

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Provide a User-Agent; some servers reject requests without one
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "eng-hackathon/1.0 (curl)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        // Network/transfer errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return "2"; // non-zero for libcurl failure
        }

        // Treat any non-2xx HTTP status as an application-level error
        if (http_code < 200 || http_code >= 300) {
            std::cerr << "HTTP error: " << http_code << "\n";
            std::cerr << "Response body (may contain HTML/login page):\n" << readBuffer << "\n";
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return "1"; // non-zero for HTTP error
        }

        // std::cout << "Response:\n" << readBuffer << "\n";
        return readBuffer;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main() {
    std::string garbage = searchCanvas("");
    std::cout << garbage << std::endl;
    std::cout << "\n\n\n";

    auto j = nlohmann::json::parse(garbage);

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

    return 0;
}