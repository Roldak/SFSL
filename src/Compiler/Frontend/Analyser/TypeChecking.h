//
//  TypeChecking.h
//  SFSL
//
//  Created by Romain Beguet on 30.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__TypeChecking__
#define __SFSL__TypeChecking__

#include <iostream>
#include <set>
#include "../AST/Visitors/ASTImplicitVisitor.h"
#include "../Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class TypeChecker : public ASTImplicitVisitor {
public:
    TypeChecker(CompCtx_Ptr& ctx, const sym::SymbolResolver& res);
    virtual ~TypeChecker();

protected:

    const sym::SymbolResolver& _res;
    common::AbstractReporter& _rep;
};

/**
 * @brief
 */
class TopLevelTypeChecking : public TypeChecker {
public:
    TopLevelTypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res);
    virtual ~TopLevelTypeChecking();

    virtual void visit(ASTNode* node) override;

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* def) override;

    virtual void visit(FunctionCreation* func) override;

private:

    Expression* _nextDef;
};

/**
 * @brief
 */
class TypeChecking : public TypeChecker {
public:

    TypeChecking(CompCtx_Ptr& ctx, const sym::SymbolResolver& res);
    virtual ~TypeChecking();

    virtual void visit(ASTNode*) override;

    virtual void visit(Program* prog) override;

    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(ExpressionStatement* exp) override;

    virtual void visit(BinaryExpression* bin) override;
    virtual void visit(AssignmentExpression* aex) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Identifier* ident) override;
    virtual void visit(This* ths) override;
    virtual void visit(BoolLitteral* boollit) override;
    virtual void visit(IntLitteral* intlit) override;
    virtual void visit(RealLitteral* reallit) override;

private:

    typedef sym::Symbolic<sym::Symbol>::SymbolData AnySymbolicData;

    struct FieldInfo final {
        FieldInfo(sym::Symbol* sy, type::Type* ty);

        bool isValid() const;

        sym::Symbol* s;
        type::Type* t;
    };

    struct ExpectedInfo final {
        const std::vector<type::Type*>* args;
        type::Type* ret;
        ASTNode* node;
    };

    FieldInfo tryGetFieldInfo(MemberAccess* dot, ClassDecl* clss, const std::string& id, const type::SubstitutionTable& subtable);

    type::Type* tryGetTypeOfSymbol(sym::Symbol* sym);
    type::ProperType* applySubsitutions(type::ProperType* inner, type::ProperType* obj);
    type::TypeConstructorType* applySubsitutions(type::TypeConstructorType* inner, type::ProperType* obj);

    sym::DefinitionSymbol* findOverridenSymbol(sym::DefinitionSymbol* def);

    template<typename SymbolIterator>
    AnySymbolicData resolveOverload(
            ASTNode* triggerer, const SymbolIterator& begin, const SymbolIterator& end, const type::SubstitutionTable& subtable);

    TypeExpression* _currentThis;
    Expression* _nextDef;

    ExpectedInfo _expectedInfo;

    std::set<DefineDecl*> _visitedDefs;
    std::vector<DefineDecl*> _redefs;
};

}

}

#endif
