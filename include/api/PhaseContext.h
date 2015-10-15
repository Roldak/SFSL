//
//  PhaseContext.h
//  SFSL
//
//  Created by Romain Beguet on 04.10.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_PhaseContext__
#define __SFSL__API_PhaseContext__

#include <iostream>
#include <map>
#include "IOPhaseObject.h"
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC PhaseContext final {
public:
    PhaseContext(const PhaseContext& other) = delete;
    PhaseContext& operator=(const PhaseContext& other) = delete;
    ~PhaseContext();

    template<typename T, typename std::enable_if<std::is_base_of<IOPhaseObject, T>::value>::type>
    T* require(const std::string& name) const;

    PhaseContext& output(const std::string& name, IOPhaseObject* object);

private:
    friend class Compiler;

    PhaseContext();

    IOPhaseObject* findIOPhaseObject(const std::string& name) const;
    void setIOPhaseObject(const std::string& name, IOPhaseObject* obj);

    std::map<std::string, IOPhaseObject*> _phaseObjects;
};

template<typename T, typename std::enable_if<std::is_base_of<IOPhaseObject, T>::value>::type>
T* PhaseContext::require(const std::string& name) const {
    if (IOPhaseObject* obj = findIOPhaseObject(name)) {
        if (obj->getId() == priv::getIOPhaseObjectID<const T*>()) {
            return obj;
        }
    }
    return nullptr;
}

}

#endif
