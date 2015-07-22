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
#include "../../Utils/Utils.h"

namespace sfsl {

namespace bc {

    /**
     * @brief Represents an abstract bytecode instruction
     */
    class BCInstruction : public common::MemoryManageable, public common::Positionnable {
    public:

        virtual ~BCInstruction();

        /**
         * @brief Appends a string representation of the bytecode instruction
         * to the given output stream
         * @param o The output stream to fill
         */
        virtual void toString(std::ostream& o) const = 0;

        /**
         * @return a string representation of the token with details
         */
        std::string toStringDetailed() const;
    };

    inline std::ostream& operator <<(std::ostream& o, const BCInstruction& i) {
        i.toString(o);
        return o;
    }

    class PushConstInt : public BCInstruction {
    public:
        PushConstInt(sfsl_int_t val);
        virtual ~PushConstInt();

        virtual void toString(std::ostream& o) const override;

    private:
        sfsl_int_t _val;
    };

    class PushConstReal : public BCInstruction {
    public:
        PushConstReal(sfsl_real_t val);
        virtual ~PushConstReal();

        virtual void toString(std::ostream& o) const override;

    private:
        sfsl_real_t _val;
    };
}

}

#endif
