//
//  CanUseModules.h
//  SFSL
//
//  Created by Romain Beguet on 28.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CanUseModules__
#define __SFSL__CanUseModules__

#include <iostream>
#include <vector>

namespace sfsl {

namespace ast {

class CanUseModules {
public:
    typedef std::vector<std::string> ModulePath;

    virtual ~CanUseModules();

    void setUsedModules(const std::vector<ModulePath>& usedModules);

    const std::vector<ModulePath>& getUsedModules() const;

private:

    std::vector<ModulePath> _usedModules;
};

}

}

#endif
