//
//  PrimitiveNamer.h (duplicate of src/Common/PrimitiveNamer.h
//  SFSL
//
//  Created by Romain Beguet on 02.01.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__PrimitiveNamer__
#define __SFSL__PrimitiveNamer__

#include <iostream>
#include <vector>
#include "SetVisibilities.h"

namespace sfsl {

namespace common {

class SFSL_API_PUBLIC PrimitiveNamer {
public:

    typedef std::vector<std::string> Path;

    virtual ~PrimitiveNamer();

    virtual Path Unit() const = 0;
    virtual Path Bool() const = 0;
    virtual Path Int() const = 0;
    virtual Path Real() const = 0;
    virtual Path String() const = 0;
    virtual Path Func(size_t nbArgs) const = 0;

private:
};

}

}

#endif
