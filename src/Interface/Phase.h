//
//  Phase.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Phase__
#define __SFSL__API_Phase__

#include <iostream>

namespace sfsl {

class Phase {
public:
    Phase(const std::string& name, const std::string& descr);
    virtual ~Phase();

    virtual void run() = 0;

private:
};

}

#endif
