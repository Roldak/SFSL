//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "UserDataAssignment.h"
#include "../../Frontend/AST/Visitors/ASTTypeCreator.h"

namespace sfsl {

namespace ast {

// ASSIGN USER DATAS

UserDataAssignment::UserDataAssignment(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _freshId(0), _currentVarCount(0) {

}

UserDataAssignment::~UserDataAssignment() {

}

void UserDataAssignment::visit(ASTNode*) {

}

void UserDataAssignment::visit(TypeDecl* tdecl) {
    ASTImplicitVisitor::visit(tdecl);
    sym::TypeSymbol* tsym = tdecl->getSymbol();

    tsym->setUserdata(_mngr.New<DefUserData>(nameFromSymbol(tsym), visibilityFromAnnotable(tdecl)));
}

void UserDataAssignment::visit(ClassDecl* clss) {
    if (TRY_INSERT(_visitedClasses, clss)) {
        for (TypeDecl* tdecl : clss->getTypeDecls()) {
            tdecl->onVisit(this);
        }

        std::vector<sym::VariableSymbol*> fields;
        std::vector<sym::DefinitionSymbol*> defs;

        if (clss->getParent()) {
            clss->getParent()->onVisit(this);

            type::ProperType* parent = static_cast<type::ProperType*>(ASTTypeCreator::createType(clss->getParent(), _ctx)->apply(_ctx));
            parent->getClass()->onVisit(this);

            ClassUserData* parentUD = parent->getClass()->getUserdata<ClassUserData>();

            fields = parentUD->getFields();
            defs = parentUD->getDefs();
        }

        SAVE_MEMBER_AND_SET(_currentVarCount, fields.size())

        for (TypeSpecifier* tps : clss->getFields()) {
            tps->onVisit(this);
            sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
            var->getUserdata<VarUserData>()->setIsAttribute(true);
            fields.push_back(var);
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

        clss->setUserdata(_mngr.New<ClassUserData>(freshName(clss->getName()), false, fields, defs, clss->isAbstract()));
    }
}

void UserDataAssignment::visit(DefineDecl* decl) {
    ASTImplicitVisitor::visit(decl);
    sym::DefinitionSymbol* def = decl->getSymbol();

    if (def->type()->getTypeKind() == type::TYPE_METHOD) {
        def->setUserdata(_mngr.New<VirtualDefUserData>(nameFromSymbol(def), visibilityFromAnnotable(decl)));
    } else {
        def->setUserdata(_mngr.New<DefUserData>(nameFromSymbol(def), visibilityFromAnnotable(decl)));
    }
}

void UserDataAssignment::visit(TypeSpecifier* tps) {
    ASTImplicitVisitor::visit(tps);
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
    var->setUserdata(_mngr.New<VarUserData>(_currentVarCount++));
}

void UserDataAssignment::visit(FunctionCreation* func) {
    bool isFunction = func->type()->getTypeKind() == type::TYPE_FUNCTION;

    SAVE_MEMBER_AND_SET(_currentVarCount, isFunction ? 0 : 1)

    ASTImplicitVisitor::visit(func);

    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        pt->getClass()->onVisit(this);
    }

    func->setUserdata(_mngr.New<FuncUserData>(freshName(func->getName()), false, _currentVarCount));

    RESTORE_MEMBER(_currentVarCount)
}

std::string UserDataAssignment::freshName(const std::string& prefix) {
    return prefix + "$" + std::to_string(_freshId++);
}

std::string UserDataAssignment::nameFromSymbol(sym::Symbol* s) {
    const std::string& name = s->getAbsoluteName();
    if (name == "") {
        return freshName(name);
    } else {
        return name;
    }
}

bool UserDataAssignment::visibilityFromAnnotable(Annotable* a) {
    bool isVisible = false;
    a->matchAnnotation<>("export", [&](){ isVisible = true; });
    return isVisible;
}

// DEFINITION USER DATA

DefUserData::DefUserData(const std::string& defId, bool isVisible) : _defId(defId), _isVisible(isVisible) {

}

DefUserData::~DefUserData() {

}

const std::string& DefUserData::getDefId() const {
    return _defId;
}

bool DefUserData::isVisible() const {
    return _isVisible;
}

// CLASS USER DATA

ClassUserData::ClassUserData(const std::string& defId, bool isHidden, const std::vector<sym::VariableSymbol*>& fields, const std::vector<sym::DefinitionSymbol*>& defs, bool isAbstract)
    : DefUserData(defId, isHidden), _fields(fields), _defs(defs), _isAbstract(isAbstract) {

}

ClassUserData::~ClassUserData() {

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

bool ClassUserData::isAbstract() const {
    return _isAbstract;
}

// FUNCTION USER DATA

FuncUserData::FuncUserData(const std::string& defId, bool isHidden, size_t varCount)
    : DefUserData(defId, isHidden), _varCount(varCount) {

}

FuncUserData::~FuncUserData() {

}

size_t FuncUserData::getVarCount() const {
    return _varCount;
}

// VARIABLE USER DATA

VarUserData::VarUserData(size_t loc) : _loc(loc), _isAttriute(false) {

}

VarUserData::~VarUserData() {

}

void VarUserData::setIsAttribute(bool value) {
    _isAttriute = value;
}

size_t VarUserData::getVarLoc() const {
    return _loc;
}

bool VarUserData::isAttribute() const {
    return _isAttriute;
}

// VIRTUAL DEFINITION USER DATA

VirtualDefUserData::VirtualDefUserData(const std::string& defId, bool isHidden) : DefUserData(defId, isHidden) {

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
