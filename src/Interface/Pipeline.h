//
//  Pipeline.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Pipeline__
#define __SFSL__API_Pipeline__

#include <vector>
#include "Phase.h"

namespace sfsl {

class Pipeline final {
public:
    Pipeline();
    ~Pipeline();

private:

    std::vector<Phase*> phase;
};

}

#endif
