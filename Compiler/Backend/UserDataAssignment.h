//
//  UserDataAssignment.h
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__UserDataAssignment__
#define __SFSL__UserDataAssignment__

#include <iostream>
#include "../Frontend/AST/Visitors/ASTVisitor.h"

namespace sfsl {

namespace bc {

    using namespace ast;

    class UserDataAssignment : public ASTVisitor {
    public:

        UserDataAssignment(CompCtx_Ptr& ctx);
        virtual ~UserDataAssignment();

        virtual void visit(ASTNode*) override;

        virtual void visit(ClassDecl* clss) override;
        virtual void visit(DefineDecl* decl) override;
        virtual void visit(TypeSpecifier* tps) override;
        virtual void visit(FunctionCreation* func) override;

    private:

        size_t _currentConstCount;
        size_t _currentVarCount;
    };

    class ClassUserData final : public common::MemoryManageable {
    public:
        ClassUserData(size_t loc, size_t attrCount, size_t defCount);
        virtual ~ClassUserData();

        size_t getClassLoc() const;
        size_t getAttrCount() const;
        size_t getDefCount() const;

    private:

        size_t _loc;
        size_t _attrCount;
        size_t _defCount;
    };

    class FuncUserData final : public common::MemoryManageable {
    public:
        FuncUserData(size_t varCount);
        virtual ~FuncUserData();

        size_t getVarCount() const;

    private:

        size_t _varCount;
    };

    class VarUserData final : public common::MemoryManageable {
    public:
        VarUserData(size_t loc);
        virtual ~VarUserData();

        size_t getVarLoc() const;

    private:

        size_t _loc;
    };

    class DefUserData final : public common::MemoryManageable {
    public:
        DefUserData(size_t loc);
        virtual ~DefUserData();

        size_t getDefLoc() const;

    private:

        size_t _loc;
    };
}

}

#endif
