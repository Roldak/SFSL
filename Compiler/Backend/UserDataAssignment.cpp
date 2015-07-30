//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "UserDataAssignment.h"
#include "../Frontend/AST/Visitors/ASTTypeCreator.h"

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
    if (TRY_INSERT(_visitedClasses, clss)) {
        std::vector<sym::VariableSymbol*> fields;
        std::vector<sym::DefinitionSymbol*> defs;

        if (clss->getParent()) {
            clss->getParent()->onVisit(this);

            type::ProperType* parent = static_cast<type::ProperType*>(ASTTypeCreator::createType(clss->getParent(), _ctx)->applied(_ctx));
            parent->getClass()->onVisit(this);

            ClassUserData* parentUD = parent->getClass()->getUserdata<ClassUserData>();

            fields = parentUD->getFields();
            defs = parentUD->getDefs();
        }

        SAVE_MEMBER_AND_SET(_currentVarCount, fields.size())

        for (TypeSpecifier* tps : clss->getFields()) {
            tps->onVisit(this);
            fields.push_back(static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol()));
        }

        RESTORE_MEMBER(_currentVarCount)

        const std::vector<DefineDecl*>& localDecls(clss->getDefs());

        for (size_t i = 0; i < localDecls.size(); ++i) {
            localDecls[i]->onVisit(this);

            sym::DefinitionSymbol* defsym = localDecls[i]->getSymbol();
            size_t virtualLoc;

            if (localDecls[i]->isRedef()) {
                sym::DefinitionSymbol* overridenSym = defsym->getOverridenSymbol();
                virtualLoc = overridenSym->getUserdata<VirtualDefUserData>()->getVirtualLocation();
            } else {
                virtualLoc = defs.size();
                defs.resize(defs.size() + 1);
            }

            defsym->getUserdata<VirtualDefUserData>()->setVirtualLocation(virtualLoc);
            defs[virtualLoc] = defsym;
        }

        size_t clssLoc = _currentConstCount++;

        clss->setUserdata(_mngr.New<ClassUserData>(clssLoc, fields, defs));
    }
}

void UserDataAssignment::visit(DefineDecl* decl) {
    ASTVisitor::visit(decl);
    sym::DefinitionSymbol* def = decl->getSymbol();

    if (def->type()->getTypeKind() == type::TYPE_METHOD) {
        def->setUserdata(_mngr.New<VirtualDefUserData>(_currentConstCount++));
    } else {
        def->setUserdata(_mngr.New<DefUserData>(_currentConstCount++));
    }
}

void UserDataAssignment::visit(TypeSpecifier* tps) {
    ASTVisitor::visit(tps);
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
    var->setUserdata(_mngr.New<VarUserData>(_currentVarCount++));
}

void UserDataAssignment::visit(FunctionCreation* func) {
    SAVE_MEMBER_AND_SET(_currentVarCount, func->type()->getTypeKind() == type::TYPE_FUNCTION ? 0 : 1)

    ASTVisitor::visit(func);

    func->setUserdata(_mngr.New<FuncUserData>(_currentVarCount));

    RESTORE_MEMBER(_currentVarCount)
}

// CLASS USER DATA

ClassUserData::ClassUserData(size_t loc, const std::vector<sym::VariableSymbol*>& fields, const std::vector<sym::DefinitionSymbol*>& defs)
    : _loc(loc), _fields(fields), _defs(defs) {

}

ClassUserData::~ClassUserData() {

}

size_t ClassUserData::getClassLoc() const {
    return _loc;
}

size_t ClassUserData::getAttrCount() const {
    return _fields.size();
}

size_t ClassUserData::getDefCount() const {
    return _defs.size();
}

const std::vector<sym::VariableSymbol*>& ClassUserData::getFields() const {
    return _fields;
}

const std::vector<sym::DefinitionSymbol*>& ClassUserData::getDefs() const {
    return _defs;
}

bool ClassUserData::indexOf(sym::VariableSymbol* field, size_t* index) const {
    for (size_t i = 0; i < _fields.size(); ++i) {
        if (_fields[i] == field) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool ClassUserData::indexOf(sym::DefinitionSymbol* def, size_t* index) const {
    for (size_t i = 0; i < _defs.size(); ++i) {
        if (_defs[i] == def) {
            *index = i;
            return true;
        }
    }
    return false;
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

// VIRTUAL DEFINITION USER DATA

VirtualDefUserData::VirtualDefUserData(size_t loc) : DefUserData(loc) {

}

VirtualDefUserData::~VirtualDefUserData() {

}

void VirtualDefUserData::setVirtualLocation(size_t virtLoc) {
    _virtLoc = virtLoc;
}

size_t VirtualDefUserData::getVirtualLocation() const {
    return _virtLoc;
}

}

}
