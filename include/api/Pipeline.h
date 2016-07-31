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
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC Pipeline final {
public:
    ~Pipeline();

    Pipeline& insert(std::shared_ptr<Phase> phase);
    Pipeline& remove(const std::string& byName);

    static Pipeline createEmpty();
    static Pipeline createDefault();

private:
    friend class Compiler;

    Pipeline();

    std::set<std::shared_ptr<Phase>> getPhases() const;

    std::set<std::shared_ptr<Phase>> _phases;
};

}

#endif
