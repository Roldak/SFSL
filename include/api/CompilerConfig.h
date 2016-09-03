//
//  CompilerConfig.h
//  SFSL
//
//  Created by Romain Beguet on 18.09.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__API_CompilerConfig__
#define __SFSL__API_CompilerConfig__

#include <iostream>
#include <memory>
#include <map>
#include <functional>
#include "SetVisibilities.h"
#include "AbstractReporter.h"
#include "AbstractPrimitiveNamer.h"
#include "CompilerOption.h"

namespace sfsl {

class SFSL_API_PUBLIC CompilerConfig final {
    typedef std::unique_ptr<void, std::function<void(void*)>> Optval;

    struct get_helper final {
        template<int i, typename Value>
        static void assign(const Value& value) {}

        template<int i, typename Value, typename Arg, typename ...Args>
        static void assign(const Value& value, Arg& arg, Args&... args) {
            arg = std::get<i>(value);
            get_helper::assign<i + 1, Value, Args...>(value, args...);
        }
    };

public:

    template<typename Opt, typename ...Args>
    typename std::enable_if<std::is_constructible<typename Opt::Params, Args...>::value, CompilerConfig&>::type
    with(Args... args) {
        static std::function<void(void*)> deleter = [](void* obj) {
            delete static_cast<typename Opt::Params*>(obj);
        };
        _options[Opt::getName()] = Optval(new typename Opt::Params(args...), deleter);
        return *this;
    }

    template<typename Opt, typename ...Args>
    typename std::enable_if<std::is_constructible<typename Opt::Params, Args...>::value, bool>::type
    get(Args&... args) const {
        auto it = _options.find(Opt::getName());
        if (it != _options.end()) {
            const auto& value = *static_cast<typename Opt::Params*>(it->second.get());
            get_helper::assign<0>(value, args...);
            return true;
        }
        return false;
    }

private:

    std::map<std::string, Optval> _options;
};

}

#endif
