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

    void setAnnotations(const std::vector<Annotation*>& annotations);
    const std::vector<Annotation*>& getAnnotations() const;

private:

    std::vector<Annotation*> _annotations;
};

}

}

#endif
