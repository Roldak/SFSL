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

struct Captures final : public common::MemoryManageable {
    Captures() : initalizerMeth(nullptr) {}
    virtual ~Captures() {}

    Identifier* initalizerMeth;
    std::vector<std::pair<Identifier*, Identifier*>> capturesData;
};

struct MutationInfo final : public common::MemoryManageable {
    MutationInfo(bool isMutable) : isMutable(isMutable), classFieldCapture(nullptr) {}
    virtual ~MutationInfo() {}

    bool isMutable;
    sym::VariableSymbol* classFieldCapture;
};

// CAPTURES ANALYZER

PreTransformAnalysis::PreTransformAnalysis(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

PreTransformAnalysis::~PreTransformAnalysis() {

}

void PreTransformAnalysis::visit(ClassDecl* clss) {
    SAVE_MEMBER_AND_SET(_usedVars, {})
    SAVE_MEMBER_AND_SET(_boundVars, {})

    /* Do as ASTImplicitVisitor::visit(clss), but don't visit TypeSpecifiers' Identifiers,
     * so that they are dealt with in the following for loop.
     */
    if (clss->getParent()) {
        clss->getParent()->onVisit(this);
    }
    for (TypeDecl* tdecl: clss->getTypeDecls()) {
        tdecl->onVisit(this);
    }
    for (TypeSpecifier* field : clss->getFields()) {
        field->getTypeNode()->onVisit(this);
    }
    for (DefineDecl* def : clss->getDefs()) {
        def->onVisit(this);
    }

    for (sym::VariableSymbol* var : _boundVars) {
        _usedVars.erase(var);
    }

    // Take into account this class's fields + inherited fields
    for (const std::pair<std::string, sym::SymbolData>& data : clss->getScope()->getAllSymbols()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(data.second.symbol)) {
            if (_usedVars.erase(var) > 0) {
                if (MutationInfo* info = var->getUserdata<MutationInfo>()) {
                    /* This will modify all the VariableSymbols associated to a capture,
                     * because they all share the same MutationInfo object. */
                    info->classFieldCapture = var;
                    info->isMutable = true;
                    MutationInfo* newInfo = _mngr.New<MutationInfo>(false);
                    newInfo->classFieldCapture = var;

                    var->setUserdata(newInfo);
                }
            }
        }
    }

    Captures* captures = _mngr.New<Captures>();

    // If there are captures
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

            MutationInfo* info = _mngr.New<MutationInfo>(_mutatedVars.find(freeVar.first) != _mutatedVars.end());
            freeVar.first->setUserdata(info);
            captureSym->setUserdata(info);
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

void PreTransformAnalysis::visit(AssignmentExpression* aex) {
    ASTImplicitVisitor::visit(aex);

    if (Identifier* ident = getIfNodeOfType<Identifier>(aex->getLhs(), _ctx)) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(ident->getSymbol())) {
            _mutatedVars.insert(var);
        }
    }
}

void PreTransformAnalysis::visit(TypeSpecifier* tps) {
    if (tps->getSpecified()->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(tps->getSpecified()->getSymbol())) {
            _boundVars.push_back(var);
        }
    }

    tps->getTypeNode()->onVisit(this);
}

void PreTransformAnalysis::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        pt->getClass()->onVisit(this);
    } else {
        ASTImplicitVisitor::visit(func);
    }
}

void PreTransformAnalysis::visit(Identifier* ident) {
    if (ident->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(ident->getSymbol())) {
            _usedVars[var].push_back(ident);
        }
    }
}

// PRETRANSFORM

PreTransformImplementation::PreTransformImplementation(
        CompCtx_Ptr& ctx,
        const common::AbstractPrimitiveNamer& namer, const sym::SymbolResolver& res)
    : ASTTransformer(ctx) {

    if (
            (_boxSymbol = sym::getIfSymbolOfType<sym::TypeSymbol>(res.getSymbol(namer.Box()))) &&
            (_boxType = type::getIf<type::TypeConstructorType>(res.Box())))
    {
        if (type::ProperType* pt = boxOf(res.Int() /*for example, not important*/)) {
            if ((_boxValueFieldSym = pt->getClass()->getScope()->getSymbol<sym::VariableSymbol>("value", false))) {
                _boxValueFieldIdent = _mngr.New<Identifier>("value");
                _boxValueFieldIdent->setSymbol(_boxValueFieldSym);
            } else {
                _ctx->reporter().fatal(*pt->getClass(), "Class Box should contain a field `value`, but it does not");
            }
        } else {
            _ctx->reporter().fatal(*_boxType->getTypeConstructor(), "Box[T] should be a proper type, but it is not");
        }
    } else {
        _ctx->reporter().fatal(common::Positionnable(0, 0, src::InputSourceName()), "Box type was not found");
    }
}

PreTransformImplementation::~PreTransformImplementation() {

}

void PreTransformImplementation::visit(ClassDecl* clss) {
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

void PreTransformImplementation::visit(MemberAccess* dot) {
    update(dot,
           transform<Expression>(dot->getAccessed()),
           dot->getMember());

    if (isLocalMutableVar(dot)) {
        makeAccessToBoxedValueOf(dot);
    } else if (sym::VariableSymbol* field = isMutableClassField(dot)) {
        makeAccessToClassField(dot, field);
    }
}

void PreTransformImplementation::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        Instantiation* inst = make<Instantiation>(pt->getClass());
        inst->setType(ASTTypeCreator::createType(inst->getInstantiatedExpression(), _ctx));

        transform<Expression>(inst);
    } else {
        ASTTransformer::visit(func);
    }
}

void PreTransformImplementation::visit(TypeSpecifier* tps) {
    update(tps, tps->getSpecified(), transform<TypeExpression>(tps->getTypeNode()));

    if (isLocalMutableVar(tps->getSpecified())) {
        type::ProperType* boxOfT = boxOf(tps->type());
        TypeIdentifier* tid = make<TypeIdentifier>("Box");
        tid->setSymbol(_boxSymbol);
        Instantiation* inst = make<Instantiation>(tid);
        inst->setType(boxOfT);

        makeAccessToBoxedValueOf(make<AssignmentExpression>(tps, inst));
    }
}

void PreTransformImplementation::visit(Instantiation* inst) {
    ASTTransformer::visit(inst);

    if (type::ProperType* tp = type::getIf<type::ProperType>(inst->type()->applyTCCallsOnly(_ctx))) {
        Captures* captures = tp->getClass()->getUserdata<Captures>();

        if (Identifier* initMeth = captures->initalizerMeth) {
            MemberAccess* dot = make<MemberAccess>(inst, initMeth);
            dot->setSymbol(initMeth->getSymbol());
            dot->setType(initMeth->type());

            std::vector<Expression*> args;
            for (std::pair<Identifier*, Identifier*> capture : captures->capturesData) {
                if (isArgumentToMutableClassField(capture.second)) {
                    args.push_back(make<This>());
                } else {
                    args.push_back(capture.second);
                }
            }

            FunctionCall* call = make<FunctionCall>(dot, nullptr, make<Tuple>(args));
            call->setType(inst->type());
        }
    }
}

void PreTransformImplementation::visit(Identifier* ident) {
    ASTTransformer::visit(ident);

    if (isLocalMutableVar(ident)) {
        makeAccessToBoxedValueOf(ident);
    } else if (sym::VariableSymbol* field = isMutableClassField(ident)) {
        makeAccessToClassField(ident, field);
    }
}

type::ProperType* PreTransformImplementation::boxOf(type::Type* tp) {
    type::ConstructorApplyType apply(_boxType, {tp});
    return type::getIf<type::ProperType>(apply.apply(_ctx));
}

bool PreTransformImplementation::isLocalMutableVar(const sym::Symbolic<sym::Symbol>* symbolic) const {
    if (symbolic->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbolic->getSymbol())) {
            if (MutationInfo* info = var->getUserdata<MutationInfo>()) {
                return info->isMutable && info->classFieldCapture == nullptr;
            }
        }
    }
    return false;
}

sym::VariableSymbol* PreTransformImplementation::isMutableClassField(const sym::Symbolic<sym::Symbol>* symbolic) const {
    if (symbolic->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbolic->getSymbol())) {
            if (MutationInfo* info = var->getUserdata<MutationInfo>()) {
                if (info->isMutable) {
                    return info->classFieldCapture;
                }
            }
        }
    }
    return nullptr;
}

bool PreTransformImplementation::isArgumentToMutableClassField(const sym::Symbolic<sym::Symbol>* symbolic) const {
    if (symbolic->getSymbol()) {
        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbolic->getSymbol())) {
            if (MutationInfo* info = var->getUserdata<MutationInfo>()) {
                if (!info->isMutable) {
                    return info->classFieldCapture;
                }
            }
        }
    }
    return false;
}

void PreTransformImplementation::makeAccessToBoxedValueOf(Expression* expr) {
    MemberAccess* dot = make<MemberAccess>(expr, _boxValueFieldIdent);
    dot->setSymbol(_boxValueFieldSym);
    dot->setType(expr->type());
}

void PreTransformImplementation::makeAccessToClassField(Expression* expr, sym::VariableSymbol* field) {
    Identifier* ident = make<Identifier>(field->getName());
    MemberAccess* dot = make<MemberAccess>(expr, ident);
    dot->setSymbol(field);
    ident->setSymbol(field);
    dot->setType(expr->type());
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
