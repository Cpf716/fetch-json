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
        std::map<std::string, std::string> headers
    ) {
        this->_status = status;
        this->_status_text = status_text;
        this->_text = text;
        this->_headers = headers;
    }

    response::response(
        const size_t status,
        const std::string status_text,
        std::map<std::string, std::string> headers,
        const std::string text
    ) {
        this->_status = status;
        this->_status_text = status_text;
        this->_headers = headers;
        this->_text = text;
    }

    response::~response() {
        delete this->_json;
    }

    // Member Functions

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
        if (this->_json == NULL) {
            if (!starts_with(this->_headers["content-type"], "application/json"))
                throw json::error("undefined");

            this->_json = json::parse(this->text());
        }

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
        // 1 - Map start line
        // 1.1 - Map method
        std::stringstream ss(method + " ");

        // Set cursor to the end
        ss.seekp(0, std::ios::end);

        // 1.2 - Map target (path)
        // 1.2.1 - Parse url
        // 1.2.1.1 - Find host
        int start = 0;

        while (start < (int)url.length() - 1 && (url[start] != '/' || url[start + 1] != '/'))
            start++;
        
        start = start == url.length() - 1 ? 0 : start + 2;

        // 1.2.1.2 - Find target
        size_t end = start;

        while (end < url.length() && url[end] != '/')
            end++;

        std::string host = url.substr(start, end - start);

        // Map target
        ss << url.substr(end) << " ";

        // 1.3 - Map protocol
        ss << "HTTP/1.1\r\n";

        // 2 - Map request headers
        for (const auto& [key, value]: headers) {
            // Override content-length and host
            std::string lower_key = tolowers(key);

            if (lower_key == "content-length" || lower_key == "host") {
                headers.erase(key);
                continue;
            }

            ss << key << ": " << value << "\r\n";
        }

        // 2.1 - Map host
        headers["host"] = host;

        ss << "host: " << host << "\r\n";

        // 3 - Map body
        if (body.length()) {
            headers["content-length"] = std::to_string(body.length());

            ss << "content-length: " << body.length() << "\r\n\r\n";
            ss << body << "\r\n";
        } else
            ss << "\r\n";

#if LOGGING
       std::cout << ss.str() << std::endl;
#endif
        // 4 - Parse host
        std::vector<std::string> components;

        split(components, host, ":");

        // No port
        if (components.size() == 1)
            throw fetch::error(0, "Unknown error");
        
        if (components[0] == "localhost")
            components[0] = "127.0.0.1";
    
        // 5 - Perform fetch
        try {
            mysocket::tcp_client* client = new mysocket::tcp_client(components[0], parse_int(components[1]));

            client->send(ss.str());

            ss.str(client->recv());
        
#if LOGGING
        std::cout << ss.str() << std::endl;
#endif

            client->close();
        } catch (mysocket::error& e) {
            throw fetch::error(0, "Unknown error", e.what());
        }

        // Server disconnected
        if (ss.str().empty())
            throw fetch::error(0, "Unknown error");

        // 6 - Parse response
        std::string str;

        getline(ss, str);

        std::vector<std::string> tokens;

        ::tokens(tokens, str);

        // 6.1 - Parse status and status text
        size_t      status = stoi(tokens[1]);
        std::string status_text = tokens[2];

        // 6.1.1 - Merge status_text
        for (size_t i = 3; i < tokens.size(); i++)
            status_text += " " + tokens[i];

        // 6.2 - Parse response headers
        std::map<std::string, std::string> _headers;

        while (getline(ss, str)) {
            std::vector<std::string> header;

            split(header, str, ":");
            
            // Empty line
            if (header.size() == 1)
                break;

            // Case-insensitive
            _headers[tolowers(header[0])] = trim(header[1]);
        }

        // 6.3 - Parse response body
        std::string        text;
        std::ostringstream oss;

        while (getline(ss, text))
            oss << trim_end(text) << "\r\n";

        std::string content_length = _headers["content-length"];
        size_t      _content_length = content_length.length() ?
            stoi(content_length) :
            _headers["transfer-encoding"] == "chunked" ?
            oss.str().length() :
            0;

        text = oss.str().substr(0, _content_length);

        // 7 - Send response
        if (status < 200 || status >= 400)
            throw fetch::error(status, status_text, text, _headers);

        return fetch::response(status, status_text, _headers, text);
    }
}
