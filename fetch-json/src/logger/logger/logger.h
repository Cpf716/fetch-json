//
//  logger.h
//  fetch-json
//
//  Created by Corey Ferguson on 2/12/26.
//

#ifndef logger_h
#define logger_h

#include <iostream>

// Typedef

enum logging { LOG_NONE, LOG_SOME, LOG_MORE };

class logger {
    // Member Fields

    logging _level = LOG_SOME;

    // Member Functions

    void log(const logging level, const std::string message);
public:
    // Constructors

    logger();

    logger(const logging level);

    // Member Functions

    void     error(const std::string message);

    logging& level();

    void     more(const std::string message);

    void     some(const std::string message);
};

// Non-Member Functions

// void _write(const std::string message);

#endif /* logger_h */
