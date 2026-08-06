//
//  fetch.h
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef fetch_h
#define fetch_h

#include "socket.h"
#include "tls.h"

namespace fetch {
    // Typedef

    enum status_code {
        UNKNOWN_ERROR = 0,
        OK = 200,
        NO_CONTENT = 204,
        FOUND = 302,
        TEMPORARY_REDIRECT = 307,
        PERMANENT_REDIRECT = 308,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
    };

    struct header {
        // Typedef

        using map = std::map<std::string, header>;

        // Constructors

        header();

        header(const char* value);

        header(const int value);

        header(const std::string value);

        header(const std::vector<std::string> value);

        // Operators

        operator                 int();

        operator                 std::string();

        operator                 std::vector<std::string>();

        int                      operator=(const int value);

        std::string              operator=(const std::string value);

        std::vector<std::string> operator=(const std::vector<std::string> value);

        bool                     operator==(const char* value);

        bool                     operator==(const int value);

        bool                     operator==(const std::string value);

        bool                     operator==(const header value);

        bool                     operator!=(const char* value);

        bool                     operator!=(const int value);

        bool                     operator!=(const std::string value);

        bool                     operator!=(const header value);

        // Member Functions

        int                      int_value() const;

        std::vector<std::string> list() const;

        std::string              str() const;
    private:
        // Member Fields

        int                      _int;
        std::vector<std::string> _list;
        std::string              _str;

        // Member Functions

        int                      _set(const int value);

        std::string              _set(const std::string value);

        std::vector<std::string> _set(const std::vector<std::string> value);
    };

    struct trailer: public header {
        // Typedef

        using map = header::map;
    };

    struct abstract_response {
        // Member Functions

        /**
         * Return response header
         */
        header       get(const std::string key);

        header::map  headers();

        bool         ok() const;

        status_code  status() const;

        std::string  status_text() const;

        std::string  text() const;

        trailer::map trailers();
    protected:
        // Member Fields
        
        header::map   _headers;
        status_code   _status;
        std::string   _status_text;
        std::string   _text;
        trailer::map  _trailers;
    };

    class error: public std::exception, public abstract_response {
        // Member Fields

        std::string _what;
    public:
        // Constructors

        error(const status_code status, const std::string status_text, const std::string text = "", header::map headers = {}, trailer::map trailers = {});

        // Member Functions

        const char* what() const throw();
    };

    struct response: public abstract_response {
        // Constructors

        response();

        response(const status_code status, const std::string status_text, const std::string text);

        response(const status_code status, const std::string status_text, header::map headers = {}, const std::string text = "", trailer::map trailers = {});
    };

    class http_client {
        // Typedef

        struct pool {
            // Typedef
    
            class connection {  
                // Member Fields

                size_t           _max = INT_MAX;
                size_t           _number = 0;
                bool             _released = false;
                size_t           _timeout = 0;
                fpp::fpp_client* _value = NULL;
            public:
                // Typedef
                
                using map = std::map<std::string, connection>;

                // Constructors

                connection();

                connection(fpp::fpp_client* value);

                // Member Functions

                size_t&          max();

                // Maintains states between threads and enforces keep-alive max
                size_t&          number();

                // Is connection available for reuse?
                bool&            released();

                size_t&          timeout();

                fpp::fpp_client* value() const;
            };

            friend class http_client;

            // Constructors

            ~pool();

            // Member Functions

            size_t           close(const std::string host);
            
            void             config(const std::string host, std::function<void(connection*)> cb);

            fpp::fpp_client* get_connection(const std::string host, class url url);
        
            void             release(const std::string host, class url url);

            void             set_logging(const logging level);
        private:
            // Member Fields

            connection::map          _connections;
            logger                   _logger;
            std::mutex               _mutex;
            std::atomic<bool>        _shut_down = false;
            std::vector<std::thread> _threads;
            
            // Member Functions
            
            size_t      _close(const std::string host);
        };

        class request {
            // Typedef
            
            friend class http_client;

            // Constructors

            request(header::map& headers, const std::string url, const std::string method, const std::string body);

            // Member Fields

            std::string _message;
            url         _url;

            // Member Functions

            std::string message() const;

            url         url();
        };
        
        // Member Fields

        logger                   _logger;
        int                      _max_redirects = 20;
        pool                     _pool;
        std::vector<std::thread> _threads;
        int                      _timeout = 30;

        // Member Functions

        response _parse_response(fpp::fpp_client* client, const std::string data);
        
        response _request(header::map& headers, const std::string url, const std::string method, const std::string body, const size_t redirects, const size_t max_redirects);
    public:

        // Constructors

        ~http_client();

        // Member Functions
        
        response get(header::map& headers, const std::string url);

        void     get(header::map& headers, const std::string url, std::function<void(response, fetch::error)> cb);
        
        response head(header::map& headers, const std::string url, const std::string body = "");
        
        int&     max_redirects();
        
        response options(header::map& headers, const std::string url);
        
        response post(header::map& headers, const std::string url, const std::string body);

        void     post(header::map& headers, const std::string url, const std::string body, std::function<void(response, fetch::error)> cb);
        
        response put(header::map& headers, const std::string url, const std::string body);

        void     put(header::map& headers, const std::string url, const std::string body, std::function<void(response, fetch::error)> cb);

        response request(header::map& headers, const std::string url, const std::string method = "get", const std::string body = "");

        void     request(header::map& headers, const std::string url, const std::string method, const std::string body, std::function<void(response, fetch::error)> cb);

        void     set_logging(const logging level);
        
        int&     timeout();
    };

    // Non-Member Functions

    // auto        _lock(std::mutex& mtx, auto cb);

    std::string statusstr(const status_code status);
}

#endif /* fetch_h */
