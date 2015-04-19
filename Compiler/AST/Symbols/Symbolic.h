//
//  Symbolic.h
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Symbolic__
#define __SFSL__Symbolic__

namespace sfsl {

namespace sym {

template<typename Symbol_Type>
/**
 * @brief An interface which specifies that the implementors can be assigned a Symbol,
 * and are therefore Symbolics
 */
class Symbolic {
public:

    Symbolic() : _symbol(nullptr) {}

    virtual ~Symbolic() {}

    /**
     * @param symbol The symbol to assign to the this Symbolic
     */
    void setSymbol(Symbol_Type* symbol) {
        _symbol = symbol;
    }

    /**
     * @return The symbol that was assigned to this Symbolic
     */
    Symbol_Type* getSymbol() const {
        return _symbol;
    }

private:

    Symbol_Type* _symbol;
};

}

}

#endif
