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

#include "../../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

class CanUseModules {
public:
    class ModulePath : public common::Positionnable {
    public:

        virtual ~ModulePath();

        void push_back(const std::string& pathUnit);
        const std::string& operator[](size_t index) const;
        size_t size() const;

    private:

        std::vector<std::string> _pathUnits;
    };

    virtual ~CanUseModules();

    void setUsedModules(const std::vector<ModulePath>& usedModules);

    const std::vector<ModulePath>& getUsedModules() const;

private:

    std::vector<ModulePath> _usedModules;
};

}

}

#endif
