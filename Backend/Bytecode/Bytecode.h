//
//  Bytecode.h
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Bytecode__
#define __SFSL__Bytecode__

#include "../../Compiler/Common/MemoryManageable.h"
#include "../../Compiler/Common/Positionnable.h"

namespace sfsl {

namespace bc {

    /**
     * @brief Represents an abstract bytecode instruction
     */
    class BCInstruction : public common::MemoryManageable, public common::Positionnable {
    public:

        virtual ~BCInstruction();

        /**
         * @return a string representation of the bytecode instruction
         */
        virtual std::string toString() const = 0;

        /**
         * @return a string representation of the token with details
         */
        std::string toStringDetailed() const;
    };

    inline std::ostream& operator <<(std::ostream& o, const BCInstruction& i) {
        return o << i.toString();
    }
}

}

#endif
