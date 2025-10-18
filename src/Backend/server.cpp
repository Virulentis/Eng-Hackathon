#include "crow.h"
#include <iostream>

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
    
    std::cout << "Server running on http://localhost:8080" << std::endl;
    app.port(8080).multithreaded().run();
}