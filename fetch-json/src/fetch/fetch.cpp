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
        const size_t status,
        const std::string status_text,
        const std::string text,
        double duration,
        std::map<std::string, std::string> headers
    ) {
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_duration = duration;
        this->_headers = headers;
    }

    response::response(
        const size_t status,
        const std::string status_text,
        std::map<std::string, std::string> headers,
        const std::string text,
        double duration
    ) {
        this->_status = status;
        this->_status_text = status_text;
        this->_headers = headers;
        this->_text = text;
        this->_duration = duration;
    }

    response::~response() {
        delete this->_json;
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

    std::string error::get(const std::string key) {
        return this->_headers[key];
    }

    std::string response::get(const std::string key) {
        return this->_headers[key];
    }

    std::map<std::string, std::string> error::headers() {
        return this->_headers;
    }

    std::map<std::string, std::string> response::headers() {
        return this->_headers;
    }

    json::object* response::json() {
        if (this->_json == NULL)
            this->_json = json::parse(this->text());

        return this->_json;
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

    const char* error::what() const throw() {
        return this->_status_text.c_str();
    }

    // Non-Member Functions

    response request(std::map<std::string, std::string>& headers, const std::string url, const std::string method, const std::string body) {
        // Map start line
        std::stringstream ss(method + " ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        // Parse url
        int start = 0;

        while (start < (int)url.length() - 1 && (url[start] != '/' || url[start + 1] != '/'))
            start++;
        
        start = start == url.length() - 1 ? 0 : start + 2;

        size_t end = start;

        while (end < url.length() && url[end] != '/')
            end++;

        // Path
        ss << url.substr(end) << " HTTP/1.1\r\n";

        // Map request headers
        // Override content-length
        headers.erase("Content-Length");
        headers.erase("content-length");

        for (const auto& [key, value]: headers)
            ss << key << ": " << value << "\r\n";

        // Map body
        if (body.length()) {
            headers["content-length"] = std::to_string(body.length());

            ss << "content-length: " << body.length() << "\r\n\r\n";
            ss << body << "\r\n";
        }

#if LOGGING
       std::cout << ss.str() << std::endl;
#endif
        // Parse host
        std::vector<std::string> components;

        split(components, url.substr(start, end - start), ":");
        
        if (components[0] == "localhost")
            components[0] = "127.0.0.1";

        std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
    
        // Perform fetch
        try {
            mysocket::tcp_client* client = new mysocket::tcp_client(components[0], parse_int(components[1]));

            client->send(ss.str());

            ss.str(client->recv());
            // ss.clear();
        
#if LOGGING
        std::cout << ss.str() << std::endl;
#endif

            client->close();
        } catch (mysocket::error& e) {
            throw fetch::error(0, "Unknown error", e.what(), std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count());
        }

        double duration = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count();

        // Parse response
        std::string str;

        getline(ss, str);

        std::vector<std::string> tokens;

        ::tokens(tokens, str);

        // Parse status and status text
        size_t      status = stoi(tokens[1]);
        std::string status_text = tokens[2];

        // Merge status_text
        for (size_t i = 3; i < tokens.size(); i++)
            status_text += " " + tokens[i];

        // Parse text
        size_t content_length = 0;

        while (getline(ss, str)) {
            transform(str.begin(), str.end(), str.begin(), ::tolower);

            std::vector<std::string> pair;

            split(pair, str, ":");

            if (pair[0] == "content-length") {
                content_length = stoi(pair[1]);
                break;
            }
        }

        str = ss.str();

        std::string text = str.substr(str.length() - content_length);

        // Parse response headers
        ss.str(str.substr(0, content_length));

        ss.seekp(1, std::ios::beg);

        std::map<std::string, std::string> _headers;

        while (getline(ss, str)) {
            std::vector<std::string> pair;

            split(pair, str, ":");
            
            // EOF
            if (pair.size() == 1)
                break;

            _headers[pair[0]] = trim(pair[1]);
        }

        if (status < 200 || status >= 400)
            throw fetch::error(status, status_text, text, duration, _headers);

        return fetch::response(status, status_text, _headers, text, duration);
    }
}
