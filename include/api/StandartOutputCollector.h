//
//  StandartOutputCollector.h
//  SFSL
//
//  Created by Romain Beguet on 16.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_StandartOutputCollector__
#define __SFSL__API_StandartOutputCollector__

#include <iostream>
#include <vector>
#include "AbstractOutputCollector.h"

namespace sfsl {

class SFSL_API_PUBLIC ByteCodeCollector : public AbstractOutputCollector {
public:
    ByteCodeCollector();
    virtual ~ByteCodeCollector();

    virtual void collect(PhaseContext& pctx);

    std::vector<std::string> get() const;

private:

    std::vector<std::string> _result;
};

class SFSL_API_PUBLIC ErrorCountCollector : public AbstractOutputCollector {
public:
    ErrorCountCollector();
    virtual ~ErrorCountCollector();

    virtual void collect(PhaseContext& pctx);

    size_t get() const;

private:

    size_t _errCount;
};

}

#endif
