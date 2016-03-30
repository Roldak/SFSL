//
//  Annotation.h
//  SFSL
//
//  Created by Romain Beguet on 30.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Annotation__
#define __SFSL__Annotation__

#include <iostream>
#include <vector>
#include <functional>

#include "../../../../Utils/Utils.h"

#include "../../../Common/Positionnable.h"

namespace sfsl {

namespace ast {

class Annotation : public common::MemoryManageable, public common::Positionnable {
public:

    struct ArgumentValue final {
        ArgumentValue();
        ArgumentValue(sfsl_bool_t b);
        ArgumentValue(sfsl_int_t i);
        ArgumentValue(sfsl_real_t r);
        ArgumentValue(const std::string& s);
        ArgumentValue(const ArgumentValue& other);

        enum { K_ANNOT_BOOL, K_ANNOT_INT, K_ANNOT_REAL, K_ANNOT_STRING } tag;
        union Value {
            Value();
            ~Value();

            sfsl_bool_t b;
            sfsl_int_t i;
            sfsl_real_t r;
            std::string s;
        } value;
    };

    Annotation(const std::string& name, const std::vector<ArgumentValue>& args);
    virtual ~Annotation();

    const std::string& getName() const;
    const std::vector<ArgumentValue>& getArgs() const;

    void setUsed();
    bool isUsed() const;

private:

    std::string _name;
    std::vector<ArgumentValue> _args;
    bool _used;
};

class Annotable {
public:

    Annotable();
    virtual ~Annotable();

    template<typename ...Args, typename Func>
    void matchAnnotation(const std::string& name, Func&& callback);

    void annotate(Annotation* annot);
    void setAnnotations(const std::vector<Annotation*>& annotations);

    const std::vector<Annotation*>& getAnnotations() const;

private:

    std::vector<Annotation*> _annotations;
};

namespace annotation_utils {

template<typename T>
struct function_traits;

template<typename ...Args>
struct function_traits<std::function<void(Args...)>> {
    static const size_t nargs = sizeof...(Args);

    template<size_t i>
    using Arg_t = typename std::tuple_element<i, std::tuple<Args...>>::type;
};

template<typename T, typename std::enable_if<std::is_same<bool, T>::value, T>::type* = nullptr>
bool fromAnnotationValue(const Annotation::ArgumentValue& in, T* out) {
    if (in.tag == Annotation::ArgumentValue::K_ANNOT_BOOL) {
        *out = in.value.b;
        return true;
    }
    return false;
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
bool fromAnnotationValue(const Annotation::ArgumentValue& in, T* out) {
    if (in.tag == Annotation::ArgumentValue::K_ANNOT_INT) {
        *out = in.value.i;
        return true;
    }
    return false;
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
bool fromAnnotationValue(const Annotation::ArgumentValue& in, T* out) {
    if (in.tag == Annotation::ArgumentValue::K_ANNOT_REAL) {
        *out = in.value.r;
        return true;
    }
    return false;
}

template<typename T, typename std::enable_if<std::is_same<std::string, std::remove_const<std::remove_reference<T>>>::value, T>::type* = nullptr>
bool fromAnnotationValue(const Annotation::ArgumentValue& in, T* out) {
    if (in.tag == Annotation::ArgumentValue::K_ANNOT_STRING) {
        *out = in.value.s;
        return true;
    }
    return false;
}

template<size_t i, size_t left, typename Func, typename ...Args>
struct CallGenerator {
    static void apply(Func callback, const std::vector<Annotation::ArgumentValue>& annotargs, Args... args) {
        typedef function_traits<Func> F_t;
        typedef typename F_t::template Arg_t<i> Arg;

        Arg a;

        if (fromAnnotationValue(annotargs[i], &a)) {
            CallGenerator<i + 1, left - 1, Func, Args..., Arg>::apply(callback, annotargs, args..., a);
        }
    }
};

template<size_t i, typename Func, typename ...Args>
struct CallGenerator<i, 0, Func, Args...> {
    static void apply(Func callback, const std::vector<Annotation::ArgumentValue>& annotargs, Args... args) {
        callback(args...);
    }
};

}

template<typename ...Args, typename Func>
void Annotable::matchAnnotation(const std::string& name, Func&& callback) {
    typedef std::function<void(Args...)> StdFunc;

    for (const Annotation* annot : _annotations) {
        if (annot->getName() == name) {
            if (annot->getArgs().size() == sizeof...(Args)) {
                annotation_utils::CallGenerator<0, sizeof...(Args), StdFunc>::apply(StdFunc(callback), annot->getArgs());
            }
        }
    }
}

}

}

#endif
