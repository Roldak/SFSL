//
//  Bytecode.h
//  SFSL
//
//  Created by Romain Beguet on 22.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__Bytecode__
#define __SFSL__Bytecode__

#include "../../../Compiler/Common/MemoryManageable.h"
#include "../../../Compiler/Common/Positionnable.h"
#include "../../../Utils/Utils.h"

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
        virtual void appendTo(std::ostream& o) const = 0;

        /**
         * @return a string representation of the token with details
         */
        std::string toStringDetailed() const;
    };

    inline std::ostream& operator <<(std::ostream& o, const BCInstruction& i) {
        i.appendTo(o);
        return o;
    }

    /*
     *  BYTECODE INSTRUCTIONS
     */

    class Label : public BCInstruction {
    public:
        Label(const std::string& name);
        virtual ~Label();

        const std::string& getName() const;

        virtual void appendTo(std::ostream &o) const override;

    private:

        std::string _name;
    };

    class MakeClass : public BCInstruction {
    public:
        MakeClass(size_t attrCount, size_t defCount);
        virtual ~MakeClass();

        virtual void appendTo(std::ostream &o) const override;

    private:

        size_t _attrCount;
        size_t _defCount;
    };

    class MakeFunction : public BCInstruction {
    public:
        MakeFunction(size_t varCount, Label* end);
        virtual ~MakeFunction();

        virtual void appendTo(std::ostream& o) const override;

    private:

        size_t _varCount;
        Label* _end;
    };

    class StoreConst : public BCInstruction {
    public:
        StoreConst(size_t index);
        virtual ~StoreConst();

        virtual void appendTo(std::ostream &o) const override;

    private:

        size_t _index;
    };

    class LoadConst : public BCInstruction {
    public:
        LoadConst(size_t index);
        virtual ~LoadConst();

        virtual void appendTo(std::ostream &o) const override;

    private:

        size_t _index;
    };

    class PushConstUnit : public BCInstruction {
    public:
        PushConstUnit();
        virtual ~PushConstUnit();

        virtual void appendTo(std::ostream &o) const override;
    };

    class PushConstInt : public BCInstruction {
    public:
        PushConstInt(sfsl_int_t val);
        virtual ~PushConstInt();

        virtual void appendTo(std::ostream& o) const override;

    private:
        sfsl_int_t _val;
    };

    class PushConstReal : public BCInstruction {
    public:
        PushConstReal(sfsl_real_t val);
        virtual ~PushConstReal();

        virtual void appendTo(std::ostream& o) const override;

    private:
        sfsl_real_t _val;
    };

    class LoadStack : public BCInstruction {
    public:
        LoadStack(size_t index);
        virtual ~LoadStack();

        virtual void appendTo(std::ostream &o) const override;

    private:

        size_t _index;
    };

    class StoreStack : public BCInstruction {
    public:
        StoreStack(size_t index);
        virtual ~StoreStack();

        virtual void appendTo(std::ostream &o) const override;

    private:

        size_t _index;
    };

    class Pop : public BCInstruction {
    public:
        Pop();
        virtual ~Pop();

        virtual void appendTo(std::ostream &o) const override;
    };

    class Return : public BCInstruction {
    public:
        Return();
        virtual ~Return();

        virtual void appendTo(std::ostream &o) const override;
    };

    class IfFalse : public BCInstruction {
    public:
        IfFalse(Label* label);
        virtual ~IfFalse();

        virtual void appendTo(std::ostream &o) const override;

    private:

        Label* _label;
    };

    class Jump : public BCInstruction {
    public:
        Jump(Label* label);
        virtual ~Jump();

        virtual void appendTo(std::ostream &o) const override;

    private:

        Label* _label;
    };
}

}

#endif
