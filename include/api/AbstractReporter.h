//
//  AbstractReporter.h
//  SFSL
//
//  Created by Romain Beguet on 01.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_AbstractReporter__
#define __SFSL__API_AbstractReporter__

#include <iostream>
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC AbstractReporter {
public:
    virtual ~AbstractReporter();

    virtual void info(const std::string& sourceName, size_t start, size_t end, const std::string& message) = 0;
    virtual void warn(const std::string& sourceName, size_t start, size_t end, const std::string& message) = 0;
    virtual void error(const std::string& sourceName, size_t start, size_t end, const std::string& message) = 0;

private:
};

}

#endif
