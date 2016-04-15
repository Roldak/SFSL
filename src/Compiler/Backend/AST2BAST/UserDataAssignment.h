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

namespace sfsl {

namespace ast {

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
    std::string nameFromSymbol(sym::Symbol* s);
    bool visibilityFromAnnotable(Annotable* a);

    size_t _currentVarCount;

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
    FuncUserData(const std::string& defId, bool isVisible, size_t varCount);
    virtual ~FuncUserData();

    size_t getVarCount() const;

private:

    size_t _varCount;
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