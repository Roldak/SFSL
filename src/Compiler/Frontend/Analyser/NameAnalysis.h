//
//  NameAnalysis.h
//  SFSL
//
//  Created by Romain Beguet on 13.12.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__NameAnalysis__
#define __SFSL__NameAnalysis__

#include <iostream>
#include <set>
#include <vector>
#include "../AST/Visitors/ASTImplicitVisitor.h"

namespace sfsl {

namespace ast {

/**
 * @brief
 */
class ScopePossessorVisitor : public ASTImplicitVisitor {
protected:
    ScopePossessorVisitor(CompCtx_Ptr& ctx);
    virtual ~ScopePossessorVisitor();

    template<typename T, typename U>
    T* createSymbol(U* node);

    template<typename T, typename S>
    void initCreated(T* id, S* s);

    template<typename T>
    void setVariableSymbolicUsed(T* symbolic, bool val);

    void buildUsingsFromPaths(const CanUseModules* canUseModules);

    sym::DefinitionSymbol* createSymbol(DefineDecl* node, TypeExpression* currentThis);
    sym::TypeSymbol* createSymbol(TypeDecl* node);

    void tryAddSymbol(sym::Symbol* sym);

    void pushPathPart(const std::string& nameSymbol);
    std::string absoluteName(const std::string& symName);
    void popPathPart();

    std::vector<std::string> _symbolPath;

    sym::Scope* _curScope;
};

/**
 * @brief
 */
class ScopeGeneration : public ScopePossessorVisitor {
public:

    ScopeGeneration(CompCtx_Ptr& ctx);
    virtual ~ScopeGeneration();

    virtual void visit(Program* prog) override;

    virtual void visit(ModuleDecl* module) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;
    virtual void visit(FunctionTypeDecl* ftdecl) override;

    virtual void visit(TypeConstructorCreation* tc) override;
    virtual void visit(TypeParameter* tparam) override;

    virtual void visit(Block* block) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(TypeSpecifier* tps) override;

private:

    void createVar(Identifier* id);
    void createProperType(TypeIdentifier* id, TypeDecl* defaultType);

    void pushScope(sym::Scoped* scoped = nullptr, bool isDefScope = false);
    void popScope();

    void generateTypeParametersSymbols(const std::vector<TypeExpression*>& typeParams);

    TypeExpression* _currentThis;
};

/**
 * @brief
 */
class TypeDependencyFixation : public ASTImplicitVisitor {
public:

    TypeDependencyFixation(CompCtx_Ptr& ctx);
    virtual ~TypeDependencyFixation();

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeConstructorCreation* tc) override;
    virtual void visit(FunctionCreation* func) override;

private:

    size_t pushTypeParameters(const std::vector<TypeExpression*>& typeParams);
    void popTypeParameters(size_t pushed);

    template<typename T>
    void debugDumpDependencies(const T* param) const;

    std::vector<sym::TypeSymbol*> _parameters;
};

/**
 * @brief
 */
class SymbolAssignation : public ScopePossessorVisitor {
public:

    SymbolAssignation(CompCtx_Ptr& ctx);
    virtual ~SymbolAssignation();

    virtual void visit(ModuleDecl* mod) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;
    virtual void visit(FunctionTypeDecl* ftdecl) override;

    virtual void visit(TypeMemberAccess* tdot) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeIdentifier* tident) override;

    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(MemberAccess* mac) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Identifier* id) override;

private:

    template<typename T>
    void assignIdentifier(T* id);

    template<typename T>
    void assignMemberAccess(T* mac);

    template<typename T>
    void assignFromStaticScope(T* mac, sym::Scoped* scoped, const std::string& typeName);

    template<typename T>
    void assignFromTypeScope(T* mac, type::Type* t);

    void visitParent(ClassDecl* clss);

    void addSubtypeRelations(ClassDecl* clss, ClassDecl* parent);
    void updateSubtypeRelations(ClassDecl* clss);

    void warnForUnusedVariableInCurrentScope();

    std::set<TypeExpression*> _temporarilyVisitedTypes;
    std::set<TypeExpression*> _visitedTypes;
};

}

}

#endif
