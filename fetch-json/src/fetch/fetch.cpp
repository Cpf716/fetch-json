//
//  fetch.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"

namespace fetch {
    // Constructors

    error::error(
        std::string url,
        std::string method,
        const size_t status,
        const std::string status_text,
        const std::string text,
        double duration
    ) {
        this->_url = url;
        this->_method = method;
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_duration = duration;
    }

    response::response(
        const std::string url,
        const std::string method,
        std::map<std::string, std::string> request_headers,
        const size_t status,
        const std::string status_text,
        const std::string text,
        std::map<std::string, std::string> response_headers,
        double duration
    ) {
        this->_url = url;
        this->_method = method;
        this->_request_headers = request_headers;
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_response_headers = response_headers;
        this->_duration = duration;
    }

    response::~response() {
        if (this->_json != NULL)
            this->_json->free();
    }

    // Member Functions

    /**
     * Return the request duration (milliseconds)
     */
    double error::duration() const {
        return this->_duration;
    }

    /**
     * Return the request duration (milliseconds)
     */
    double response::duration() const {
        return this->_duration;
    }

    json::object* response::json() {
        if (this->_json == NULL)
            this->_json = json::parse(this->text());

        return this->_json;
    }

    std::string error::method() const {
        return this->_method;
    }

    std::string response::method() const {
        return this->_method;
    }

    std::map<std::string, std::string> response::request_headers() {
        return this->_request_headers;
    }

    std::map<std::string, std::string> response::response_headers() {
        return this->_response_headers;
    }

    size_t error::status() const {
        return this->_status;
    }

    size_t response::status() const {
        return this->_status;
    }

    std::string error::status_text() const {
        return this->_status_text;
    }

    std::string response::status_text() const {
        return this->_status_text;
    }

    std::string error::text() const {
        return this->_text;
    }

    std::string response::text() const {
        return this->_text;
    }

    std::string error::url() const {
        return this->_url;
    }

    std::string response::url() const {
        return this->_url;
    }

    const char* error::what() const throw() {
        return this->_status_text.c_str();
    }

    // Non-Member Functions

    response request(const std::string url, const std::string method, const std::string body, std::map<std::string, std::string> headers) {
        std::stringstream ss("curl -vs -X ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        ss << method << " ";

        if (body.length())
            ss << "-d " << encode(body) << " ";

        for (const auto& [key, value]: headers)
            ss << "-H " << encode(key + ": " + value) << " ";

        // path()
        std::string response = "/tmp/" + uuid(),
                    error = "/tmp/" + uuid();

        ss << url << " ";
        ss << "> " << response << " 2> " << error;

        // class logger
        // std::cout << ss.str() << std::endl;

        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    
        system(ss.str().c_str());

        double duration = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

        ss.str("");
        ss.clear();

        std::ifstream file(response);

        if (file.is_open()) {
            ss << file.rdbuf();

            // Perform garbage collection
            auto free = [&file](const std::string filename) {
                file.close();

                std::remove(filename.c_str());
            };

            free(response);
            
            std::string text = ss.str();

            ss.str("");
            ss.clear();
            
            file = std::ifstream(error);
            
            if (file.is_open()) {
                // Map request headers
                std::string str;

                while (getline(file, str) && str[0] != '>')
                    continue;

                if (file.peek() == EOF) {
                    free(error);

                    throw fetch::error(url, method, 0, "Unknown Error", "", duration);
                }

                auto map_headers = [&](std::map<std::string, std::string>& map) {
                    while (getline(file, str)) {
                        std::vector<std::string> tokens;
                        
                        ::tokens(tokens, str);
                        
                        if (tokens.size() == 1)
                            break;
                        
                        for (size_t i = 3; i < tokens.size() - 1; i++)
                            ss << tokens[i] << " ";
                        
                        ss << tokens[tokens.size() - 1];
                        
                        std::string value = ss.str();
                        
                        map[tokens[1].substr(0, tokens[1].length() - 1)] = is_string_literal(value) ? encode(value) : value;
                        
                        ss.str("");
                        ss.clear();
                    }
                };

                std::map<std::string, std::string> request_headers;

                map_headers(request_headers);

                // Map response headers
                while (getline(file, str) && str[0] != '<')
                    continue;

                std::vector<std::string> tokens;

                ::tokens(tokens, str);
                
                size_t status = stoi(tokens[2]);

                for (size_t i = 3; i < tokens.size(); i++)
                    ss << tokens[i] << " ";

                std::string status_text = ss.str();

                if (status >= 200 && status < 400) {
                    ss.str("");
                    ss.clear();

                    std::map<std::string, std::string> response_headers;
                    
                    map_headers(response_headers);

                    free(error);
                    
                    return fetch::response(url, method, request_headers, status, status_text, text, response_headers, duration);
                }

                free(error);

                throw fetch::error(url, method, status, status_text, text, duration);
            }
            
            throw fetch::error(url, method, 0, "Unknown error", std::strerror(errno), duration);
        }

        throw fetch::error(url, method, 0, "Unknown error", std::strerror(errno), duration);
    }
}
