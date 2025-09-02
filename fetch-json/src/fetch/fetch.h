//
//  fetch.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef fetch_h
#define fetch_h

#include "json.h"
#include "util.h"
#include <fstream>

namespace fetch {
    // Typedef

    struct error: public std::exception {
        // Constructors

        error(
            std::string       url,
            std::string       method,
            const size_t      status,
            const std::string status_text,
            const std::string text,
            double            duration
        );

        // Member Functions

        double      duration() const;

        std::string method() const;

        size_t      status() const;

        std::string status_text() const;

        std::string text() const;

        std::string url() const;

        const char* what() const throw();
    private:
        // Member Fields

        double      _duration;
        std::string _method;
        size_t      _status;
        std::string _status_text;
        std::string _text;
        std::string _url;
    };

    struct response {
        // Constructors

        response(
            std::string                        url,
            std::string                        method,
            std::map<std::string, std::string> request_headers,
            const size_t                       status,
            const std::string                  status_text,
            const std::string                  text,
            std::map<std::string, std::string> response_headers,
            double                             duration
        );

        ~response();

        // Member Functions

        double                             duration() const;

        json::object*                      json();

        std::string                        method() const;

        std::map<std::string, std::string> request_headers();

        std::map<std::string, std::string> response_headers();

        size_t                             status() const;

        std::string                        status_text() const;

        std::string                        text() const;

        std::string                        url() const;
    private:
        double                             _duration;
        json::object*                      _json = NULL;
        std::string                        _method;
        std::map<std::string, std::string> _request_headers;
        std::map<std::string, std::string> _response_headers;
        size_t                             _status;
        std::string                        _status_text;
        std::string                        _text;
        std::string                        _url;
    };

    // Non-Member Functions
    
    response request(
        const std::string                  url,
        const std::string                  method = "GET", 
        const std::string                  body = "", 
        std::map<std::string, std::string> headers = std::map<std::string, std::string>()
    );
}

#endif /* fetch_h */
