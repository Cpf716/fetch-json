#include "fpp_client.h"

namespace fpp {
    // Constructors

    error::error(const std::string what) {
        this->_what = what;
    }
    
    // Member Functions

    const char* error::what() const throw() {
        return this->_what.c_str();
    }
}