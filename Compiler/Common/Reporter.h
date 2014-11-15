//
//  Reporter.h
//  SFSL
//
//  Created by Romain Beguet on 15.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Reporter__
#define __SFSL__Reporter__

#include <iostream>
#include "Positionnable.h"

namespace sfsl {

namespace common {

    /**
     * @brief Abstract representing an Reporter which is used throughout the compilation process
     * to reporter the different errors to the users. See implementations
     */
    class AbstractReporter {
    public:

        virtual ~AbstractReporter();

        /**
         * @brief Used to report informations to the user
         * @param pos The origin of the information
         * @param msg The message describing the information
         */
        virtual void info(const Positionnable& pos, const std::string& msg) = 0;

        /**
         * @brief Used to report warnings to the user
         * @param pos The origin of the warning
         * @param msg The message describing the warning
         */
        virtual void warning(const Positionnable& pos, const std::string& msg) = 0;

        /**
         * @brief Used to report errors to the user
         * @param pos The origin of the error
         * @param msg The message describing the error
         */
        virtual void error(const Positionnable& pos, const std::string& msg) = 0;

        /**
         * @brief Used to report fatal errors to the user
         * (generally, internal errors/bug/missing features of the compiler itself)
         *
         * @param pos The origin of the error
         * @param msg The message describing the fatal error
         */
        virtual void fatal(const Positionnable& pos, const std::string& msg) = 0;
    };

    class StandartErrReporter : public AbstractReporter {
    public:

        StandartErrReporter();
        virtual ~StandartErrReporter();

        virtual void info(const Positionnable &pos, const std::string &msg);

        virtual void warning(const Positionnable &pos, const std::string &msg);

        virtual void error(const Positionnable &pos, const std::string &msg);

        virtual void fatal(const Positionnable &pos, const std::string &msg);

    private:

        void reportMessage(const std::string& prefix, const Positionnable& pos, const std::string& msg);

    };

}

}

#endif
