//
//  fetch.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef fetch_h
#define fetch_h

#include "json.h"
#include "socket.h"
#include <fstream>

namespace fetch {
    // Typedef

    struct response_t {
        // Member Functions

        /**
         * Return response header
         */
        virtual std::string                        get(const std::string key) = 0;

        virtual std::map<std::string, std::string> headers() = 0;

        virtual size_t                             status() const = 0;

        virtual std::string                        status_text() const = 0;

        virtual std::string                        text() const = 0;
    };

    struct error: public std::exception, public response_t {
        // Constructors

        error(
            const size_t                       status,
            const std::string                  status_text,
            const std::string                  text,
            std::map<std::string, std::string> headers = {}
        );

        // Member Functions

        /**
         * Return response header
         */
        std::string                        get(const std::string key);

        std::map<std::string, std::string> headers();

        size_t                             status() const;

        std::string                        status_text() const;

        std::string                        text() const;

        const char*                        what() const throw();
    private:
        // Member Fields

        std::map<std::string, std::string> _headers;
        size_t                             _status;
        std::string                        _status_text;
        std::string                        _text;
    };

    struct response: public response_t {
        // Constructors

        response(
            const size_t                       status,
            const std::string                  status_text,
            std::map<std::string, std::string> headers,
            const std::string                  text
        );

        ~response();

        // Member Functions

        /**
         * Return response header
         */
        std::string                        get(const std::string key);

        json::object*                      json();

        std::map<std::string, std::string> headers();

        size_t                             status() const;

        std::string                        status_text() const;

        std::string                        text() const;
    private:
        // Member Fields
        
        std::map<std::string, std::string> _headers;
        json::object*                      _json = NULL;
        size_t                             _status;
        std::string                        _status_text;
        std::string                        _text;
    };

    // Non-Member Functions
    
    response request(
        std::map<std::string, std::string>& headers,
        const std::string                   url,
        const std::string                   method = "GET",
        const std::string                   body = ""
    );
}

#endif /* fetch_h */
