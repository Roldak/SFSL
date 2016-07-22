//
//  AbstractPrimitiveNamer.h
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AbstractPrimitiveNamer__
#define __SFSL__AbstractPrimitiveNamer__

#include <iostream>
#include <vector>

namespace sfsl {

namespace common {

class AbstractPrimitiveNamer {
public:

    typedef std::vector<std::string> Path;

    virtual ~AbstractPrimitiveNamer();

    virtual Path Unit() const = 0;
    virtual Path Bool() const = 0;
    virtual Path Int() const = 0;
    virtual Path Real() const = 0;
    virtual Path String() const = 0;
    virtual Path Box() const = 0;
    virtual Path Func(size_t nbArgs) const = 0;

private:
};

}

}

#endif
