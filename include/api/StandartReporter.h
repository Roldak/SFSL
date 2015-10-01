//
//  StandartReporter.h
//  SFSL
//
//  Created by Romain Beguet on 01.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_StandartReporter__
#define __SFSL__API_StandartReporter__

#include <iostream>
#include "AbstractReporter.h"

namespace sfsl {

class SFSL_API_PUBLIC StandartReporter : public AbstractReporter {
public:
    StandartReporter(std::ostream& stream);
    virtual ~StandartReporter();

    virtual void info(const std::string& sourceName, size_t start, size_t end, const std::string& message) override;
    virtual void warning(const std::string& sourceName, size_t start, size_t end, const std::string& message) override;
    virtual void error(const std::string& sourceName, size_t start, size_t end, const std::string& message) override;
    virtual void fatal(const std::string& sourceName, size_t start, size_t end, const std::string& message) override;

    static StandartReporter* const CoutReporter;
    static StandartReporter* const CerrReporter;
    static AbstractReporter* const EmptyReporter;

private:

    std::ostream& _ostream;
};

}

#endif
