//
//  CompilationTest.h
//  SFSL
//
//  Created by Romain Beguet on 30.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CompilationTest__
#define __SFSL__CompilationTest__

#include <iostream>

#include "sfsl.h"
#include "AbstractTest.h"

namespace sfsl {

namespace test {

class CompilationTest : public AbstractTest {
public:
    CompilationTest(const std::string& name, Compiler& cmp, const std::string& source, bool shouldCompile);
    virtual ~CompilationTest();

    virtual bool run(AbstractTestLogger& logger) override;

private:

    void buildSTDModules(ProgramBuilder builder);

    Compiler& _cmp;
    const std::string _source;
    bool _shouldCompile;
};

}

}

#endif
