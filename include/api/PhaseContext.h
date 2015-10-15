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
#include "Errors.h"
#include "SetVisibilities.h"

namespace sfsl {

class SFSL_API_PUBLIC PhaseContext final {
public:
    PhaseContext(const PhaseContext& other) = delete;
    PhaseContext& operator=(const PhaseContext& other) = delete;
    ~PhaseContext();

    template<typename T>
    T* require(const std::string& name) const;

    template<typename T>
    PhaseContext& output(const std::string& name, T* object);

private:
    friend class Compiler;

    PhaseContext();

    IOPhaseObject* findIOPhaseObject(const std::string& name) const;
    void setIOPhaseObject(const std::string& name, IOPhaseObject* obj);

    std::map<std::string, IOPhaseObject*> _phaseObjects;
};

template<typename T>
T* PhaseContext::require(const std::string& name) const {
    IOPhaseObject* obj = findIOPhaseObject(name);
    if (obj->getId() == priv::getIOPhaseObjectID<const priv::IOPhaseObjectWrapper<T>*>()) {
        return ((priv::IOPhaseObjectWrapper<T>*)obj)->get();
    } else {
        throw CompileError("Phase object '" + name + "' does not match the required type.");
    }
}

template<typename T>
PhaseContext& PhaseContext::output(const std::string& name, T* object) {
    priv::IOPhaseObjectWrapper<T>* wrapper = new priv::IOPhaseObjectWrapper<T>(object);
    setIOPhaseObject(name, wrapper);
    return *this;
}

}

#endif
