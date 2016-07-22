//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "UserDataAssignment.h"
#include "../../Frontend/AST/Visitors/ASTTypeCreator.h"
#include "../../Frontend/Symbols/Scope.h"

namespace sfsl {

namespace ast {

// CAPTURES

Captures::Captures() : initalizerMeth(nullptr) {

}

Captures::~Captures() {

}

// CAPTURES ANALYZER

CapturesAnalyzer::CapturesAnalyzer(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

CapturesAnalyzer::~CapturesAnalyzer() {

}

void CapturesAnalyzer::visit(ClassDecl* clss) {
    SAVE_MEMBER_AND_SET(_usedVars, {})
    SAVE_MEMBER_AND_SET(_boundVars, {})

    ASTImplicitVisitor::visit(clss);

    for (sym::VariableSymbol* var : _boundVars) {
        _usedVars.erase(var);
    }

    // to take into account inherited fields
    for (const std::pair<std::string, sym::SymbolData>& data : clss->getScope()->getAllSymbols()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(data.second.symbol)) {
            _usedVars.erase(var);
        }
    }

    Captures* captures = _mngr.New<Captures>();

    if (_usedVars.size() > 0) {
        const std::string initializerName = clss->getName() + "$init";
        std::vector<Expression*> initExprs;
        std::vector<Expression*> initArgs;

        for (const std::pair<sym::VariableSymbol*, std::vector<Identifier*>>& freeVar : _usedVars) {
            Identifier* newField = _mngr.New<Identifier>(freeVar.first->getName());
            Identifier* fieldArg = _mngr.New<Identifier>(newField->getValue() + "$arg");

            sym::VariableSymbol* captureSym = _mngr.New<sym::VariableSymbol>(newField->getValue(), "");
            newField->setSymbol(captureSym);

            sym::VariableSymbol* argSym = _mngr.New<sym::VariableSymbol>(fieldArg->getValue(), "");
            fieldArg->setSymbol(argSym);

            for (Identifier* ident : freeVar.second) {
                ident->setSymbol(captureSym);
            }

            Identifier* instantiationArg = _mngr.New<Identifier>(newField->getValue());
            instantiationArg->setSymbol(freeVar.first);
            captures->capturesData.push_back(std::make_pair(newField, instantiationArg));

            __old_usedVars[freeVar.first].push_back(instantiationArg);

            initExprs.push_back(_mngr.New<AssignmentExpression>(newField, fieldArg));
            initArgs.push_back(_mngr.New<TypeSpecifier>(fieldArg, _mngr.New<TypeToBeInferred>()));
        }

        initExprs.push_back(_mngr.New<This>());

        FunctionCreation* func = _mngr.New<FunctionCreation>(
                    initializerName, nullptr, _mngr.New<Tuple>(initArgs), _mngr.New<Block>(initExprs));

        func->setType(_mngr.New<type::MethodType>(
                          clss,
                          std::vector<TypeExpression*>(),
                          std::vector<type::Type*>(),
                          nullptr, type::Environment::Empty));

        Identifier* initIdent = _mngr.New<Identifier>(initializerName);
        DefineDecl* initDef = _mngr.New<DefineDecl>(initIdent, nullptr, func, false, false, false);

        sym::DefinitionSymbol* initSym = _mngr.New<sym::DefinitionSymbol>(initIdent->getValue(), "", initDef, clss);
        initIdent->setSymbol(initSym);
        initDef->setSymbol(initSym);

        initIdent->setType(func->type());
        initSym->setType(func->type());

        captures->initalizerMeth = initIdent;
    }


    clss->setUserdata<Captures>(captures);

    RESTORE_MEMBER(_boundVars)
    RESTORE_MEMBER(_usedVars)
}

void CapturesAnalyzer::visit(TypeSpecifier* tps) {
    if (tps->getSpecified()->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(tps->getSpecified()->getSymbol())) {
            _boundVars.push_back(var);
        }
    }

    tps->getTypeNode()->onVisit(this);
}

void CapturesAnalyzer::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        pt->getClass()->onVisit(this);
    } else {
        ASTImplicitVisitor::visit(func);
    }
}

void CapturesAnalyzer::visit(Identifier* ident) {
    if (ident->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(ident->getSymbol())) {
            _usedVars[var].push_back(ident);
        }
    }
}

// HANDLE CAPTURES

PreTransform::PreTransform(CompCtx_Ptr& ctx) : ASTTransformer(ctx) {

}

PreTransform::~PreTransform() {

}

void PreTransform::visit(ClassDecl* clss) {
    Captures* captures = clss->getUserdata<Captures>();

    TypeExpression* parent = transform<TypeExpression>(clss->getParent());
    std::vector<TypeDecl*> types(transform<TypeDecl>(clss->getTypeDecls()));
    std::vector<TypeSpecifier*> fields(transform<TypeSpecifier>(clss->getFields()));
    std::vector<DefineDecl*> decls(transform<DefineDecl>(clss->getDefs()));

    for (std::pair<Identifier*, Identifier*> capture : captures->capturesData) {
        fields.push_back(make<TypeSpecifier>(capture.first, make<TypeToBeInferred>()));
    }

    if (Identifier* initMeth = captures->initalizerMeth) {
        decls.push_back(static_cast<sym::DefinitionSymbol*>(initMeth->getSymbol())->getDef());
    }

    update(clss, clss->getName(), parent, types, fields, decls, clss->isAbstract());
}

void PreTransform::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        Instantiation* inst = make<Instantiation>(pt->getClass());
        inst->setType(ASTTypeCreator::createType(inst->getInstantiatedExpression(), _ctx));

        transform<Expression>(inst);
    } else {
        ASTTransformer::visit(func);
    }
}

void PreTransform::visit(Instantiation* inst) {
    ASTTransformer::visit(inst);

    if (type::ProperType* tp = type::getIf<type::ProperType>(inst->type()->applyTCCallsOnly(_ctx))) {
        Captures* captures = tp->getClass()->getUserdata<Captures>();

        if (Identifier* initMeth = captures->initalizerMeth) {
            MemberAccess* dot = make<MemberAccess>(inst, initMeth);
            dot->setSymbol(initMeth->getSymbol());
            dot->setType(initMeth->type());

            std::vector<Expression*> args;
            for (std::pair<Identifier*, Identifier*> capture : captures->capturesData) {
                args.push_back(capture.second);
            }

            FunctionCall* call = make<FunctionCall>(dot, nullptr, make<Tuple>(args));
            call->setType(inst->type());
        }
    }
}

// ASSIGN USER DATAS

UserDataAssignment::UserDataAssignment(CompCtx_Ptr& ctx)
    : ASTImplicitVisitor(ctx), _freshId(0), _currentVarCount(0), _nextConstructorExpr(nullptr) {

}

UserDataAssignment::~UserDataAssignment() {

}

void UserDataAssignment::visit(TypeDecl* tdecl) {
    ASTImplicitVisitor::visit(tdecl);
    sym::TypeSymbol* tsym = tdecl->getSymbol();

    tsym->setUserdata(_mngr.New<DefUserData>(nameFromSymbol(tsym), visibilityFromAnnotable(tdecl)));
}

void UserDataAssignment::visit(ClassDecl* clss) {
    if (TRY_INSERT(_visitedClasses, clss)) {
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

        for (TypeDecl* tdecl : clss->getTypeDecls()) {
            tdecl->onVisit(this);
        }
    }
}

void UserDataAssignment::visit(DefineDecl* decl) {
    SAVE_MEMBER(_nextConstructorExpr)

    if (decl->getName()->getValue() == "new") {
        _nextConstructorExpr = decl->getValue();
    }

    ASTImplicitVisitor::visit(decl);
    sym::DefinitionSymbol* def = decl->getSymbol();

    if (def->type()->getTypeKind() == type::TYPE_METHOD) {
        def->setUserdata(_mngr.New<VirtualDefUserData>(nameFromSymbol(def), visibilityFromAnnotable(decl)));
    } else {
        def->setUserdata(_mngr.New<DefUserData>(nameFromSymbol(def), visibilityFromAnnotable(decl)));
    }

    RESTORE_MEMBER(_nextConstructorExpr)
}

void UserDataAssignment::visit(TypeSpecifier* tps) {
    ASTImplicitVisitor::visit(tps);
    sym::VariableSymbol* var = static_cast<sym::VariableSymbol*>(tps->getSpecified()->getSymbol());
    var->setUserdata(_mngr.New<VarUserData>(_currentVarCount++));
}

void UserDataAssignment::visit(FunctionCreation* func) {
    SAVE_MEMBER_AND_SET(_currentVarCount, 1)

    ASTImplicitVisitor::visit(func);

    func->setUserdata(_mngr.New<FuncUserData>(freshName(func->getName()), false, _currentVarCount, _nextConstructorExpr == func));

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

// ANNOTATION USAGE WARNER

AnnotationUsageWarner::AnnotationUsageWarner(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx), _rep(ctx->reporter()) {

}

AnnotationUsageWarner::~AnnotationUsageWarner() {

}

void AnnotationUsageWarner::visit(ModuleDecl* module) {
    visitAnnotable(module);
    ASTImplicitVisitor::visit(module);
}

void AnnotationUsageWarner::visit(TypeDecl* tdecl) {
    visitAnnotable(tdecl);
    ASTImplicitVisitor::visit(tdecl);
}

void AnnotationUsageWarner::visit(ClassDecl* clss) {
    visitAnnotable(clss);
    ASTImplicitVisitor::visit(clss);
}

void AnnotationUsageWarner::visit(DefineDecl* decl) {
    visitAnnotable(decl);
    ASTImplicitVisitor::visit(decl);
}

void AnnotationUsageWarner::visit(FunctionCreation* func) {
    visitAnnotable(func);
    ASTImplicitVisitor::visit(func);
}

void AnnotationUsageWarner::visitAnnotable(Annotable* annotable) {
    for (Annotation* annot : annotable->getAnnotations()) {
        if (!annot->isUsed()) {
            _rep.warning(*annot, "Unused annotation. It is either ill-formed, or the plugin using these annotations is missing.");
        }
    }
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

FuncUserData::FuncUserData(const std::string& defId, bool isHidden, size_t varCount, bool isConstructorExpression)
    : DefUserData(defId, isHidden), _varCount(varCount), _isConstructorExpression(isConstructorExpression) {

}

FuncUserData::~FuncUserData() {

}

size_t FuncUserData::getVarCount() const {
    return _varCount;
}

bool FuncUserData::isConstructorExpression() const {
    return _isConstructorExpression;
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
