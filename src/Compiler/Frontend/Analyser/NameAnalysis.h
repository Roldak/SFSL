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
class ScopeGeneration : public ASTImplicitVisitor {
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

    void generateTypeParametersSymbols(const std::vector<TypeExpression*>& typeParams, bool allowVarianceAnnotations);

    template<typename T, typename U>
    T* createSymbol(U* node);
    sym::DefinitionSymbol* createSymbol(DefineDecl* node, TypeExpression* currentThis);
    sym::TypeSymbol* createSymbol(TypeDecl* node);

    template<typename T, typename S>
    void initCreated(T* id, S* s);

    void tryAddSymbol(sym::Symbol* sym);

    void pushPathPart(const std::string& nameSymbol, bool becomesInvalid);
    bool isValidAbsolutePath() const;
    std::string absoluteName(const std::string& symName);
    void popPathPart();
    void reportPotentiallyInvalidExternUsage(const common::Positionnable& pos) const;

    sym::Scope* _curScope;

    std::vector<std::string> _symbolPath;
    size_t _invalidFrom;

    TypeExpression* _currentThis;
    FunctionTypeDecl* _nextMethodDecl;
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

    typedef type::TypeParametrizable::Parameter Parameter;

    size_t pushTypeParameters(const std::vector<TypeExpression*>& typeParams);
    void popTypeParameters(size_t pushed);

    template<typename T>
    void debugDumpDependencies(const T* param) const;

    std::vector<Parameter> _parameters;
};

/**
 * @brief
 */
class SymbolAssignation : public ASTImplicitVisitor {
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
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Identifier* id) override;

private:

    void buildUsingsFromPaths(const CanUseModules* canUseModules);

    template<typename T>
    void assignIdentifier(T* id);

    template<typename T>
    void assignMemberAccess(T* mac);

    template<typename T>
    void assignFromStaticScope(T* mac, sym::Scoped* scoped, const std::string& typeName);

    template<typename T>
    void assignFromTypeScope(T* mac, type::Type* t);

    bool visitParent(ClassDecl* clss);

    sym::Scope* _curScope;

    std::set<TypeExpression*> _temporarilyVisitedTypes;
    std::set<TypeExpression*> _visitedTypes;
};

}

}

#endif
