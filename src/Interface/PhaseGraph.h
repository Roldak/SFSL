//
//  PhaseGraph.h
//  SFSL
//
//  Created by Romain Beguet on 12.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_PhaseGraph__
#define __SFSL__API_PhaseGraph__

#include <set>
#include <memory>
#include <stdexcept>

#include "../../include/api/Phase.h"
#include "../../include/api/SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC PhaseGraphResolutionError : public std::runtime_error {
public:
    PhaseGraphResolutionError(const std::string& err);
    virtual ~PhaseGraphResolutionError();
};

std::vector<std::shared_ptr<Phase>> SFSL_API_PUBLIC sortPhases(const std::set<std::shared_ptr<Phase>>& phases);

}

#endif
