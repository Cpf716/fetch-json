//
//  fpp_client.h
//  fetch-json
//
//  Created by Corey Ferguson on 7/10/26.
//

#ifndef fpp_client_h
#define fpp_client_h

#define BUFF_LEN 65536 // 64 KB

#include <iostream>

namespace fpp {
    struct fpp_client {
        virtual void        close() = 0;

        virtual std::string recv() = 0;

        virtual int         send(const std::string message) = 0;
    };

    struct error: public std::exception {
        // Constructors
        
        error(const std::string what);
        
        // Member Fields

        const char* what() const throw();
    private:
        // Member Fields

        std::string _what;
    };
}

#endif /* fpp_client_h */
