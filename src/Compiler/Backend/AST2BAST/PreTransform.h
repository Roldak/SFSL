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
#include "../../Frontend/AST/Visitors/ASTImplicitVisitor.h"
#include "../../Frontend/AST/Visitors/ASTTransformer.h"
#include "../../Frontend/Symbols/SymbolResolver.h"

namespace sfsl {

namespace ast {

class PreTransformAnalysis : public ASTImplicitVisitor {
public:
    PreTransformAnalysis(CompCtx_Ptr& ctx);
    virtual ~PreTransformAnalysis();

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(Identifier* ident) override;

private:
    void visitClassDecl(ClassDecl* clss);
    ClassDecl* getParentMostClass(ClassDecl* clss);

    std::map<sym::VariableSymbol*, std::vector<Identifier*>> _usedVars;
    std::vector<sym::VariableSymbol*> _boundVars;

    std::map<ClassDecl*, sym::VariableSymbol*> _classThisSymbols;
};

class PreTransformImplementation : public ASTTransformer {
public:
    PreTransformImplementation(CompCtx_Ptr& ctx, const common::AbstractPrimitiveNamer& namer, const sym::SymbolResolver& res);
    virtual ~PreTransformImplementation();

    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Instantiation* inst) override;
    virtual void visit(Identifier* ident) override;

private:

    type::ProperType* boxOf(type::Type* tp);

    bool isCapturedClassField(Identifier* ident) const;
    bool isCapturedLocalMutableVar(sym::Symbol* symbol) const;
    bool isClassThis(sym::Symbol* symbol) const;

    Expression* makeBoxInstantiationOf(type::Type* tp);
    Expression* makeAccessToCapturedClassField(Identifier* ident);
    Expression* makeAccessToBoxedValueOf(Expression* expr);
    This* makeAccessToClassThis();

    Expression* transformFuncBody(Expression* oldBody, Expression* args);

    type::TypeConstructorType* _boxType;
    sym::TypeSymbol* _boxSymbol;
    sym::VariableSymbol* _boxValueFieldSym;
    Identifier* _boxValueFieldIdent;

    std::map<Identifier*, sym::Symbol*> _curCapturedFields;
};

/**
 * @brief
 */
class UserDataAssignment : public ASTImplicitVisitor {
public:

    UserDataAssignment(CompCtx_Ptr& ctx);
    virtual ~UserDataAssignment();

    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;

    virtual void visit(DefineDecl* decl) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(FunctionCreation* func) override;

private:

    size_t _freshId;
    std::string freshName(const std::string& prefix);
    std::string nameFromSymbol(sym::Symbol* s, bool isVisible);
    std::string nameFromDefSymbol(sym::DefinitionSymbol* s, bool isVisible, bool& signaturePrinterSuccess);
    bool visibilityFromAnnotable(Annotable* a);

    size_t _currentVarCount;
    Expression* _nextConstructorExpr;

    std::set<ClassDecl*> _visitedClasses;
};

class AnnotationUsageWarner : public ASTImplicitVisitor {
public:

    AnnotationUsageWarner(CompCtx_Ptr& ctx);
    virtual ~AnnotationUsageWarner();

    virtual void visit(ModuleDecl* module) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;

    virtual void visit(DefineDecl* decl) override;
    virtual void visit(FunctionCreation* func) override;

private:

    void visitAnnotable(Annotable* annotable);

    common::AbstractReporter& _rep;
};

struct Change {
    Change(Identifier* nf, Identifier* ia);

    sym::VariableSymbol* getNewFieldSymbol() const;

    Identifier* newField;
    Identifier* initializerArg;
};

class ClassPatch final : public common::MemoryManageable {
public:
    ClassPatch(Identifier* initializer, const std::vector<Change>& changes,
               const std::map<Identifier*, sym::Symbol*>& fieldCaptures);

    virtual ~ClassPatch();

    Identifier* getInitializer() const;
    const std::vector<Change>& getChanges() const;
    const std::map<Identifier*, sym::Symbol*>& getFieldCaptures() const;

private:

    Identifier* _initalizer;
    std::vector<Change> _changes;
    std::map<Identifier*, sym::Symbol*> _fieldCaptures;
};

class DefUserData : public common::MemoryManageable {
public:
    DefUserData(const std::string& defId, bool isVisible);
    virtual ~DefUserData();

    const std::string& getDefId() const;

    bool isVisible() const;

private:

    const std::string _defId;
    bool _isVisible;
};

class ClassUserData final : public DefUserData {
public:
    ClassUserData(const std::string& defId, bool isVisible, const std::vector<sym::VariableSymbol*>& fields, const std::vector<sym::DefinitionSymbol*>& defs, bool isAbstract);
    virtual ~ClassUserData();

    size_t getAttrCount() const;
    size_t getDefCount() const;

    const std::vector<sym::VariableSymbol*>& getFields() const;
    const std::vector<sym::DefinitionSymbol*>& getDefs() const;

    bool indexOf(sym::VariableSymbol* field, size_t* index) const;
    bool indexOf(sym::DefinitionSymbol* def, size_t* index) const;

    bool isAbstract() const;

private:

    std::vector<sym::VariableSymbol*> _fields;
    std::vector<sym::DefinitionSymbol*> _defs;
    bool _isAbstract;
};

class FuncUserData final : public DefUserData {
public:
    FuncUserData(const std::string& defId, bool isVisible, size_t varCount, bool isConstructorExpression);
    virtual ~FuncUserData();

    size_t getVarCount() const;
    bool isConstructorExpression() const;

private:

    size_t _varCount;
    bool _isConstructorExpression;
};

class VarUserData final : public common::MemoryManageable {
public:
    VarUserData(size_t loc);
    virtual ~VarUserData();

    void setIsAttribute(bool value);

    size_t getVarLoc() const;
    bool isAttribute() const;

private:

    size_t _loc;
    bool _isAttriute;
};

class VirtualDefUserData : public DefUserData {
public:
    VirtualDefUserData(const std::string& defId, bool isVisible);
    virtual ~VirtualDefUserData();

    void setVirtualLocation(size_t virtLoc);

    size_t getVirtualLocation() const;

private:

    size_t _virtLoc;
};

}

}

#endif
