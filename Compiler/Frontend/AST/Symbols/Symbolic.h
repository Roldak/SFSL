//
//  Symbolic.h
//  SFSL
//
//  Created by Romain Beguet on 20.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Symbolic__
#define __SFSL__Symbolic__

#include <vector>
#include <map>

namespace sfsl {

namespace type {
    class Type;
    typedef std::map<Type*, Type*> SubstitutionTable;
}

namespace sym {

template<typename Symbol_Type>
/**
 * @brief An interface which specifies that the implementors can be assigned a Symbol,
 * and are therefore Symbolics
 */
class Symbolic {
    friend class Scope;

    struct SymbolData {
        Symbol_Type* symbol;
        const type::SubstitutionTable* env;
    };

public:

    Symbolic() : _symbols({SymbolData{.symbol = nullptr, .env = nullptr}}) {}

    virtual ~Symbolic() {}

    /**
     * @param symbol The symbol to assign to the this Symbolic
     */
    void setSymbol(Symbol_Type* symbol) {
        _symbols[0] = SymbolData{.symbol = symbol, .env = nullptr};
    }

    /**
     * @return The symbol that was assigned to this Symbolic
     */
    Symbol_Type* getSymbol() const {
        return _symbols[0].symbol;
    }

    /**
     * @return The number of symbols that was assigned to this Symbolic
     */
    size_t getSymbolCount() const {
        return _symbols.size();
    }

    /**
     * @return Every SymbolDatas that this symbolic holds
     */
    const std::vector<SymbolData>& getSymbolDatas() {
        return _symbols;
    }

private:

    std::vector<SymbolData> _symbols;
};

}

}

#endif
