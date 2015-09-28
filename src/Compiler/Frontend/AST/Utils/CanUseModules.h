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

/**
 * @brief An interface to store the module paths that
 * an object is using
 */
class CanUseModules {
public:
    /**
     * @brief A module path
     */
    class ModulePath : public common::Positionnable {
    public:

        virtual ~ModulePath();

        /**
         * @brief Adds an path unit to this path. For example, the path "sfsl.lang"
         * is composed of two path units: "sfsl" and "lang".
         *
         * @param pathUnit The path unit to add to this path
         */
        void push_back(const std::string& pathUnit);

        /**
         * @param index The index at which to retrieve the path unit
         * @return The path unit that was retrieved from the given index
         */
        const std::string& operator[](size_t index) const;

        /**
         * @return The number of path units that this path contains
         */
        size_t size() const;

        /**
         * @return A string representation of the path
         */
        std::string toString() const;

        /**
         * @param toElemIndex The index of the last element to add to the string
         * @return A string representation of the path up to the path unit at the given index.
         */
        std::string toString(size_t toElemIndex) const;

    private:

        std::vector<std::string> _pathUnits;
    };

    virtual ~CanUseModules();

    /**
     * @param usedModules The vector containing the "used" modules to assign to this object
     */
    void setUsedModules(const std::vector<ModulePath>& usedModules);

    /**
     * @return The vector containing the modules that this object is using
     */
    const std::vector<ModulePath>& getUsedModules() const;

private:

    std::vector<ModulePath> _usedModules;
};

}

}

#endif
