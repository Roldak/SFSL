//
//  CanSubtype.h
//  SFSL
//
//  Created by Romain Beguet on 24.08.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__CanSubtype__
#define __SFSL__CanSubtype__

#include <algorithm>
#include <vector>
#include "Environment.h"

namespace sfsl {

namespace type {

template<typename Type>
class CanSubtype {
private:

    struct Entry {
        Entry(Type t, const std::vector<Environment>& e) : tpe(t), envs(e) {}

        inline bool operator <(Type t) const {
            return tpe < t;
        }

        inline bool correspondsTo(Type t) const {
            return tpe == t;
        }

        inline void addInstance(const Environment& env) {
            envs.push_back(env);
        }

        inline Type getType() const {
            return tpe;
        }

        inline const std::vector<Environment>& getInstances() const {
            return envs;
        }

    private:

        Type tpe;
        std::vector<Environment> envs;
    };

    typedef typename std::vector<Entry>::iterator EntryIterator;

public:

    virtual ~CanSubtype() { }

    void addSuperType(Type t, const Environment& env) {
        t->addImmediateSubType(this, env);
        recursivelyAddSuperType(t, env);
    }

    void addSpecialSuperType(Type t, const Environment& env) {
        findOrAddSuperType(t)->addInstance(env);
    }

    const std::vector<Environment>& subTypeInstances(Type t) {
        static const std::vector<Environment> None = {};

        EntryIterator it = find(t);
        if (it != _superTypes.end()) {
            return it->getInstances();
        }

        return None;
    }

private:

    void recursivelyAddSuperType(Type t, const Environment& env) {
        for (const Entry& e : t->_superTypes) {
            std::vector<Environment> envs;
            for (const Environment& inst : e.getInstances()) {
                Environment subInst = inst;
                subInst.substituteAll(env);
                envs.push_back(subInst);
            }
            _superTypes.push_back(Entry(e.getType(), envs));
        }

        for (std::pair<CanSubtype<Type>*, Environment>& sub : getImmediateSubTypes()) {
            Environment substitued = env;
            substitued.substituteAll(sub.second);
            sub.first->recursivelyAddSuperType(t, substitued);
        }
    }

    void addImmediateSubType(CanSubtype<Type>* t, const Environment& env) {
        _immSubTypes.push_back(std::make_pair(t, env));
    }

    std::vector<std::pair<CanSubtype<Type>*, Environment>>& getImmediateSubTypes() {
        return _immSubTypes;
    }

    EntryIterator find(Type t) {
        EntryIterator it = std::lower_bound(_superTypes.begin(), _superTypes.end(), t);
        if (it == _superTypes.end() || !it->correspondsTo(t)) {
            return _superTypes.end();
        }
        return it;
    }

    EntryIterator findOrAddSuperType(Type t) {
        EntryIterator it = std::lower_bound(_superTypes.begin(), _superTypes.end(), t);
        if (it == _superTypes.end() || !it->correspondsTo(t)) {
            return _superTypes.insert(it, Entry(t, {}));
        }
        return it;
    }

    std::vector<Entry> _superTypes;
    std::vector<std::pair<CanSubtype<Type>*, Environment>> _immSubTypes;
};

}

}

#endif
