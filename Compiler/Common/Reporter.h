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

        virtual void warning(const Positionnable& pos, const std::string& msg) = 0;
        virtual void error(const Positionnable& pos, const std::string& msg) = 0;

    private:
    };

}

}

#endif
