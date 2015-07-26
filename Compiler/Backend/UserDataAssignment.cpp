//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "UserDataAssignment.h"

namespace sfsl {

namespace bc {

// ASSIGN USER DATAS

UserDataAssignment::UserDataAssignment(CompCtx_Ptr& ctx)
    : ASTVisitor(ctx), _currentConstCount(0), _currentVarCount(0) {

}

UserDataAssignment::~UserDataAssignment() {

}

void UserDataAssignment::visit(ASTNode*) {

}

void UserDataAssignment::visit(ClassDecl* clss) {
    ASTVisitor::visit(clss);

    size_t clssLoc = _currentConstCount++;
    size_t attrCount = clss->getFields().size();
    size_t defCount = clss->getDefs().size();

    clss->setUserdata(_mngr.New<ClassUserData>(clssLoc, attrCount, defCount));
}

void UserDataAssignment::visit(DefineDecl* decl) {
    ASTVisitor::visit(decl);
    sym::DefinitionSymbol* def = decl->getSymbol();
    def->setUserdata(_mngr.New<DefUserData>(_currentConstCount++));
}

void UserDataAssignment::visit(TypeSpecifier* tps) {
    ASTVisitor::visit(tps);
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
    var->setUserdata(_mngr.New<VarUserData>(_currentVarCount++));
}

void UserDataAssignment::visit(FunctionCreation* func) {
    SAVE_MEMBER_AND_SET(_currentVarCount, 0)

    ASTVisitor::visit(func);

    func->setUserdata(_mngr.New<FuncUserData>(_currentVarCount));

    RESTORE_MEMBER(_currentVarCount)
}

// CLASS USER DATA

ClassUserData::ClassUserData(size_t loc, size_t attrCount, size_t defCount)
    : _loc(loc), _attrCount(attrCount), _defCount(defCount) {

}

ClassUserData::~ClassUserData() {

}

size_t ClassUserData::getClassLoc() const {
    return _loc;
}

size_t ClassUserData::getAttrCount() const {
    return _attrCount;
}

size_t ClassUserData::getDefCount() const {
    return _defCount;
}

// FUNCTION USER DATA

FuncUserData::FuncUserData(size_t varCount) : _varCount(varCount) {

}

FuncUserData::~FuncUserData() {

}

size_t FuncUserData::getVarCount() const {
    return _varCount;
}

// VARIABLE USER DATA

VarUserData::VarUserData(size_t loc) : _loc(loc) {

}

VarUserData::~VarUserData() {

}

size_t VarUserData::getVarLoc() const {
    return _loc;
}

// DEFINITION USER DATA

DefUserData::DefUserData(size_t loc) : _loc(loc) {

}

DefUserData::~DefUserData() {

}

size_t DefUserData::getDefLoc() const {
    return _loc;
}

}

}
