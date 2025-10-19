#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

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
    std::string url = base_url + tail_url;

    const char* env = std::getenv("CANVAS_TOKEN");
    if (!env || std::string(env).empty()) {
        std::cerr << "CANVAS_TOKEN not set. Put it in .env and run: set -o allexport; source .env; set +o allexport\n";
        return "";
    }
    std::string token(env);

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

void ModifyCourseIDsAndNamesFromCanvas(std::vector<std::string> *course_ids, std::vector<std::string> *course_names){
    std::string readBuffer = searchCanvas("/courses");
    std::vector<std::string> ids;
    std::vector<std::string> names;
    if (readBuffer.empty()) {
        std::cerr << "Failed to fetch courses or empty response.\n";
        return;
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
                    if (idnum != 0){
                        ids.push_back(std::to_string(idnum));
                        names.push_back(name);
                    } 
                }
                // std::cout << "id=" << idnum << " name=" << name << "\n";
            }
        }
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }

    *course_ids = ids;
    *course_names = names;
}

std::string searchCanvasAssignments(){
    std::vector<std::string> course_ids;
    std::vector<std::string> course_names;
    ModifyCourseIDsAndNamesFromCanvas(&course_ids, &course_names);
    std::string output;
    for(int i=0;i<course_ids.size();i++){
        output.append("\"id\": " + course_ids.at(i) + "\"name\": " + course_names.at(i));
        std::string tail_url = "/courses/" + course_ids.at(i) + "/assignments";
        std::string readBuffer = searchCanvas(tail_url);
        // std::cout << readBuffer << std::endl; //DEBUG

        if (readBuffer.empty()) {
            std::cerr << "Empty assignments response for course " << course_ids.at(i) << "\n";
            continue;
        }

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(readBuffer);
        } catch (const nlohmann::json::parse_error &e) {
            std::cerr << "JSON parse error for course " << course_ids.at(i) << ": " << e.what() << "\n";
            continue;
        }

        if (j.is_array()) {
            for (const auto& item : j) {
                int id = 0;
                if (item.contains("id") && !item["id"].is_null()) {
                    try { id = item["id"].get<int>(); } catch (...) { id = 0; }
                }

                auto get_str_safe = [](const nlohmann::json &obj, const char* key) -> std::string {
                    if (obj.contains(key) && !obj[key].is_null()) {
                        try { return obj[key].get<std::string>(); } catch (...) { return std::string(); }
                    }
                    return std::string();
                };

                std::string name = get_str_safe(item, "name");
                std::string created_at = get_str_safe(item, "created_at");
                std::string due_at = get_str_safe(item, "due_at");

                std::ostringstream ss;
                ss << "{ \"course_id\": " << course_ids.at(i)
                   << ", \"id\": " << id
                   << ", \"name\": " << nlohmann::json(name).dump()
                   << ", \"created_at\": " << nlohmann::json(created_at).dump()
                   << ", \"due_at\": " << nlohmann::json(due_at).dump()
                   << " }";

                output.append(ss.str());
                // std::cout << ss.str() << "\n\n";
            }
        }
    }
    // std::cout << output;
    return output;
}

std::string parseOutputData(std::string output){
    nlohmann::json arr = nlohmann::json::array();
    size_t pos = 0;

    struct CourseEntry { std::string id; std::string name; };
    std::vector<CourseEntry> courseNames;

    auto trim = [](std::string &s){
        size_t a = 0;
        while (a < s.size() && std::isspace((unsigned char)s[a])) ++a;
        size_t b = s.size();
        while (b > a && std::isspace((unsigned char)s[b-1])) --b;
        s = s.substr(a, b - a);
        if (s.size() >= 2) {
            if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))
                s = s.substr(1, s.size() - 2);
        }
    };

    while (pos < output.size()) {
        // find next object start
        size_t start = output.find('{', pos);
        if (start == std::string::npos) break;

        // examine any header text between pos and start for course id/name pairs
        if (start > pos) {
            std::string header = output.substr(pos, start - pos);
            size_t pid = header.find("\"id\": ");
            if (pid != std::string::npos) {
                pid += 6; // move past "\"id\": "
                while (pid < header.size() && std::isspace((unsigned char)header[pid])) ++pid;
                size_t k = pid;
                // read optional sign and digits
                if (k < header.size() && (header[k] == '-' || std::isdigit((unsigned char)header[k]))) {
                    ++k;
                    while (k < header.size() && std::isdigit((unsigned char)header[k])) ++k;
                    std::string cid = header.substr(pid, k - pid);
                    size_t pname = header.find("\"name\": ", k);
                    if (pname != std::string::npos) {
                        pname += 8; // past "\"name\": "
                        while (pname < header.size() && std::isspace((unsigned char)header[pname])) ++pname;
                        std::string cname = header.substr(pname);
                        trim(cname);
                        if (!cid.empty() && !cname.empty()) {
                            courseNames.push_back(CourseEntry{cid, cname});
                        }
                    }
                }
            }
        }

        int depth = 0;
        size_t i = start;
        for (; i < output.size(); ++i) {
            if (output[i] == '{') ++depth;
            else if (output[i] == '}') {
                --depth;
                if (depth == 0) { ++i; break; } // include this closing brace
            }
        }
        if (depth != 0) break; // unmatched braces: stop

        std::string objstr = output.substr(start, i - start);
        try {
            auto obj = nlohmann::json::parse(objstr);

            // if we have a course_id, try to attach course_name from the parsed headers
            if (obj.contains("course_id")) {
                std::string cid;
                try {
                    if (obj["course_id"].is_string()) cid = obj["course_id"].get<std::string>();
                    else if (obj["course_id"].is_number()) cid = std::to_string(obj["course_id"].get<int>());
                } catch (...) { cid.clear(); }

                if (!cid.empty()) {
                    for (const auto &ce : courseNames) {
                        if (ce.id == cid) {
                            obj["course_name"] = ce.name;
                            break;
                        }
                    }
                }
            }

            arr.push_back(obj);
        } catch (const nlohmann::json::parse_error &e) {
            std::cerr << "parseOutputData: JSON parse error: " << e.what() << "\n";
            // skip invalid object
        }
        pos = i;
    }

    return arr.dump(4); // pretty print with 4-space indent
}

int main() {
    std::cout << parseOutputData(searchCanvasAssignments());
    std::cout << "\n";
    

    return 0;
}
