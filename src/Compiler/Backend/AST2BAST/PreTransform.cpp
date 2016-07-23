//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "PreTransform.h"
#include "../../Frontend/AST/Visitors/ASTTypeCreator.h"
#include "../../Frontend/Symbols/Scope.h"

namespace sfsl {

namespace ast {

/*
 * VARIABLE INFOS
 */

struct SymbolInfo;

typedef std::shared_ptr<SymbolInfo> SymbolInfoPtr;

/**
 * @brief A SymbolInfo is shared among several VariableInfo.
 * For example, when capturing some local variable x inside a class A, two VariableSymbol
 * (and therefore two VariableInfo) will represent the same original variable x. The one that is used
 * inside the local scope, and the one that is used in the class A. Basically, a SymbolInfo is a way to
 * link all the VariableInfos that share the same original symbol. (In the example, the two VariableInfo
 * will have the same SymbolInfo which refers to the original local variable x).
 */
struct SymbolInfo final {
    SymbolInfo() : isMutable(false) {}

    static SymbolInfoPtr make() {
        return std::make_shared<SymbolInfo>();
    }

    bool isMutable;
};

enum VARIABLE_TYPE { LOCAL, FIELD };

/**
 * @brief Base class for VariableInfos
 */
struct VariableInfo : public common::MemoryManageable {
    virtual ~VariableInfo() {}

    virtual VariableInfo* copy(CompCtx_Ptr ctx) const = 0;
    virtual VARIABLE_TYPE type() const = 0;

    bool isMutable() const {
        return symInfo->isMutable;
    }
    void setMutable() {
        symInfo->isMutable = true;
    }

    SymbolInfoPtr symInfo;

protected:
    VariableInfo(SymbolInfoPtr symInfo) : symInfo(symInfo) {}
};

/**
 * @brief Represents a local variable
 */
struct LocalInfo final : public VariableInfo {
    LocalInfo(SymbolInfoPtr symInfo) : VariableInfo(symInfo) {}
    virtual ~LocalInfo() {}

    VariableInfo* copy(CompCtx_Ptr ctx) const override {
        return ctx->memoryManager().New<LocalInfo>(symInfo);
    }

    VARIABLE_TYPE type() const override {
        return LOCAL;
    }
};

/**
 * @brief Represents a field of a class
 */
struct FieldInfo final : public VariableInfo {
    FieldInfo(SymbolInfoPtr symInfo) : VariableInfo(symInfo) {}
    virtual ~FieldInfo() {}

    VariableInfo* copy(CompCtx_Ptr ctx) const override {
        return ctx->memoryManager().New<FieldInfo>(symInfo);
    }

    VARIABLE_TYPE type() const override {
        return FIELD;
    }
};

inline VariableInfo* getVariableInfo(sym::Symbol* symbol) {
    common::HasManageableUserdata* holder = nullptr;
    if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbol)) {
        holder = var;
    } else if (sym::DefinitionSymbol* def = sym::getIfSymbolOfType<sym::DefinitionSymbol>(symbol)) {
        holder = def;
    }
    if (holder) {
        return holder->getUserdata<VariableInfo>();
    }
    return nullptr;
}

inline VariableInfo* setVariableInfo(sym::Symbol* symbol, VariableInfo* info) {
    common::HasManageableUserdata* holder = nullptr;
    if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(symbol)) {
        holder = var;
    } else if (sym::DefinitionSymbol* def = sym::getIfSymbolOfType<sym::DefinitionSymbol>(symbol)) {
        holder = def;
    }
    if (holder) {
        holder->setUserdata(info);
        return info;
    }
    return nullptr;
}

/*
 * BUILDERS
 */

struct InitializerBuilder final {
    InitializerBuilder(ClassDecl* clss, CompCtx_Ptr ctx)
        : _clss(clss), _ctx(ctx), _mngr(_ctx->memoryManager()) { }

    void addInitializationOf(Identifier* field) {
        Identifier* arg = _mngr.New<Identifier>(field->getValue() + "$arg");
        sym::VariableSymbol* argSym = _mngr.New<sym::VariableSymbol>(arg->getValue(), "");
        arg->setSymbol(argSym);

        _params.push_back(_mngr.New<TypeSpecifier>(arg, _mngr.New<TypeToBeInferred>()));
        _body.push_back(_mngr.New<AssignmentExpression>(field, arg));
    }

    Identifier* build() {
        if (_body.size() == 0) {
            return nullptr;
        }

        _body.push_back(_mngr.New<This>());

        std::string name = _clss->getName() + "$init";

        FunctionCreation* func = _mngr.New<FunctionCreation>(
                    name, nullptr, _mngr.New<Tuple>(_params), _mngr.New<Block>(_body));

        func->setType(_mngr.New<type::MethodType>(
                          _clss,
                          std::vector<TypeExpression*>(),
                          std::vector<type::Type*>(),
                          nullptr, type::Environment::Empty));

        Identifier* initIdent = _mngr.New<Identifier>(name);
        DefineDecl* initDef = _mngr.New<DefineDecl>(initIdent, nullptr, func, false, false, false);

        sym::DefinitionSymbol* initSym = _mngr.New<sym::DefinitionSymbol>(initIdent->getValue(), "", initDef, _clss);
        initIdent->setSymbol(initSym);
        initDef->setSymbol(initSym);

        initIdent->setType(func->type());
        initSym->setType(func->type());

        return initIdent;
    }

private:

    ClassDecl* _clss;
    std::vector<Expression*> _params;
    std::vector<Expression*> _body;

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
};

struct Change {
    Change(Identifier* nf, Identifier* ia) : newField(nf), initializerArg(ia) {}

    sym::VariableSymbol* getNewFieldSymbol() const {
        return sym::getIfSymbolOfType<sym::VariableSymbol>(newField->getSymbol());
    }

    Identifier* newField;
    Identifier* initializerArg;
};

struct ClassPatch final : public common::MemoryManageable {
    ClassPatch(Identifier* initializer, const std::vector<Change>& changes)
        : _initalizer(initializer), _changes(changes) {}
    virtual ~ClassPatch() {}

    Identifier* getInitializer() const {
        return _initalizer;
    }

    const std::vector<Change>& getChanges() const {
        return _changes;
    }

private:

    Identifier* _initalizer;
    std::vector<Change> _changes;
};

struct ClassPatcher final {
    ClassPatcher(ClassDecl* clss, CompCtx_Ptr& ctx)
        : _clss(clss), _initBuilder(clss, ctx), _ctx(ctx), _mngr(_ctx->memoryManager()) {}

    Change addNewField(const std::string& name) {
        // Make new field
        Identifier* newField = _mngr.New<Identifier>(name);
        sym::VariableSymbol* newFieldSym = _mngr.New<sym::VariableSymbol>(name, "");
        newField->setSymbol(newFieldSym);

        // Add entry to initializer
        _initBuilder.addInitializationOf(newField);

        // Create the argument that will be passed to the initializer
        Identifier* initializerArg = _mngr.New<Identifier>(name);

        // Create the change representation
        Change change(newField, initializerArg);
        _changes.push_back(change);
        return change;
    }

    ClassPatch* buildPatch() {
        return _mngr.New<ClassPatch>(_initBuilder.build(), _changes);
    }

private:

    ClassDecl* _clss;
    InitializerBuilder _initBuilder;
    std::vector<Change> _changes;

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
};

// CAPTURES ANALYZER

PreTransformAnalysis::PreTransformAnalysis(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

PreTransformAnalysis::~PreTransformAnalysis() {

}

void PreTransformAnalysis::visitClassDecl(ClassDecl* clss) {
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

    for (const auto& pair : clss->getScope()->getAllSymbols()) {
        if (sym::Symbol* s = pair.second.symbol) {
            if (!getVariableInfo(s)) {
                setVariableInfo(s, _mngr.New<FieldInfo>(SymbolInfo::make()));
            }
        }
    }
}

void PreTransformAnalysis::visit(ClassDecl* clss) {
    SAVE_MEMBER_AND_SET(_usedVars, {})
    SAVE_MEMBER_AND_SET(_boundVars, {})

    visitClassDecl(clss);

    for (sym::VariableSymbol* var : _boundVars) {
        _usedVars.erase(var);
    }

    ClassPatcher classPatcher(clss, _ctx);

    // For all the captures
    for (const auto& freeVar : _usedVars) {
        sym::VariableSymbol* capturedSymbol(freeVar.first);
        const std::vector<Identifier*>& referringCapturedSymbol(freeVar.second);

        // Add a new field to the class, which has the name of the captured variable.
        Change change = classPatcher.addNewField(capturedSymbol->getName());
        // Make that new field has the same variable infos
        setVariableInfo(change.getNewFieldSymbol(), getVariableInfo(capturedSymbol)->copy(_ctx));

        // For all identifiers that refer to this capture,
        // make them refer to the new class field instead.
        for (Identifier* ident : referringCapturedSymbol) {
            ident->setSymbol(change.getNewFieldSymbol());
        }

        // But make the initializer argument refer to the captured variable.
        change.initializerArg->setSymbol(capturedSymbol);
        OLD(_usedVars)[capturedSymbol].push_back(change.initializerArg);
    }

    clss->setUserdata<ClassPatch>(classPatcher.buildPatch());

    RESTORE_MEMBER(_boundVars)
    RESTORE_MEMBER(_usedVars)
}

void PreTransformAnalysis::visit(AssignmentExpression* aex) {
    ASTImplicitVisitor::visit(aex);

    if (Identifier* ident = getIfNodeOfType<Identifier>(aex->getLhs(), _ctx)) {
        getVariableInfo(ident->getSymbol())->setMutable();
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
        VariableInfo* info = getVariableInfo(ident->getSymbol());
        if (!info) {
            info = setVariableInfo(ident->getSymbol(), _mngr.New<LocalInfo>(SymbolInfo::make()));
        }
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
    ClassPatch* classPatch = clss->getUserdata<ClassPatch>();

    TypeExpression* parent = transform<TypeExpression>(clss->getParent());
    std::vector<TypeDecl*> types(transform<TypeDecl>(clss->getTypeDecls()));
    std::vector<TypeSpecifier*> fields(transform<TypeSpecifier>(clss->getFields()));
    std::vector<DefineDecl*> decls(transform<DefineDecl>(clss->getDefs()));

    // Actually add the new field
    for (Change change : classPatch->getChanges()) {
        fields.push_back(make<TypeSpecifier>(change.newField, make<TypeToBeInferred>()));
    }

    // As well as the initializer
    if (Identifier* initializer = classPatch->getInitializer()) {
        decls.push_back(static_cast<sym::DefinitionSymbol*>(initializer->getSymbol())->getDef());
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
        ClassPatch* classPatch = tp->getClass()->getUserdata<ClassPatch>();

        if (Identifier* initializer = classPatch->getInitializer()) {
            MemberAccess* dot = make<MemberAccess>(inst, initializer);
            dot->setSymbol(initializer->getSymbol());
            dot->setType(initializer->type());

            std::vector<Expression*> args;
            for (Change change : classPatch->getChanges()) {
                if (isArgumentToMutableClassField(change.initializerArg)) {
                    args.push_back(make<This>());
                } else {
                    args.push_back(change.initializerArg);
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
        if (VariableInfo* info = getVariableInfo(symbolic->getSymbol())) {
            return info->type() == LOCAL && info->isMutable();
        }
    }
    return false;
}

sym::VariableSymbol* PreTransformImplementation::isMutableClassField(const sym::Symbolic<sym::Symbol>* symbolic) const {
    return nullptr;
}

bool PreTransformImplementation::isArgumentToMutableClassField(const sym::Symbolic<sym::Symbol>* symbolic) const {
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
