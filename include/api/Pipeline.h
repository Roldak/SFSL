//
//  Pipeline.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_Pipeline__
#define __SFSL__API_Pipeline__

#include <set>
#include <memory>
#include "Phase.h"

namespace sfsl {

class Pipeline final {
public:
    Pipeline();
    ~Pipeline();

    Pipeline& insert(std::shared_ptr<Phase> phase);

private:

    std::set<std::shared_ptr<Phase>> _phases;
};

}

#endif
