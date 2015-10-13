//
//  PhaseGraph.h
//  SFSL
//
//  Created by Romain Beguet on 12.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_PhaseGraph__
#define __SFSL__API_PhaseGraph__

#include "api/Phase.h"
#include <set>
#include <memory>

namespace sfsl {

std::vector<std::shared_ptr<Phase>> sortPhases(std::set<std::shared_ptr<Phase>>& phases);

}

#endif
