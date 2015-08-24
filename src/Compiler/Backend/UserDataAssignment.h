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
#include <set>
#include "../Frontend/AST/Visitors/ASTVisitor.h"

namespace sfsl {

namespace bc {

    using namespace ast;

    /**
     * @brief
     */
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

        std::set<ClassDecl*> _visitedClasses;
    };

    class ClassUserData final : public common::MemoryManageable {
    public:
        ClassUserData(size_t loc, const std::vector<sym::VariableSymbol*>& fields, const std::vector<sym::DefinitionSymbol*>& defs);
        virtual ~ClassUserData();

        size_t getClassLoc() const;
        size_t getAttrCount() const;
        size_t getDefCount() const;

        const std::vector<sym::VariableSymbol*>& getFields() const;
        const std::vector<sym::DefinitionSymbol*>& getDefs() const;

        bool indexOf(sym::VariableSymbol* field, size_t* index) const;
        bool indexOf(sym::DefinitionSymbol* def, size_t* index) const;

    private:

        size_t _loc;
        std::vector<sym::VariableSymbol*> _fields;
        std::vector<sym::DefinitionSymbol*> _defs;
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

    class DefUserData : public common::MemoryManageable {
    public:
        DefUserData(size_t loc);
        virtual ~DefUserData();

        size_t getDefLoc() const;

    private:

        size_t _loc;
    };

    class VirtualDefUserData : public DefUserData {
    public:
        VirtualDefUserData(size_t loc);
        virtual ~VirtualDefUserData();

        void setVirtualLocation(size_t virtLoc);

        size_t getVirtualLocation() const;

    private:

        size_t _virtLoc;
    };
}

}

#endif
