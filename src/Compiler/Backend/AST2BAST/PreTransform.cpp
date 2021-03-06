//
//  UserDataAssignment.cpp
//  SFSL
//
//  Created by Romain Beguet on 23.07.15.
//  Copyright (c) 2015 Romain Beguet. All rights reserved.
//

#include "PreTransform.h"
#include "../../Frontend/AST/Visitors/ASTTypeCreator.h"
#include "../../Frontend/AST/Visitors/ASTAssignmentChecker.h"
#include "../../Frontend/AST/Visitors/ASTSignaturePrinter.h"
#include "../../Frontend/Symbols/Scope.h"

namespace sfsl {

namespace ast {

/*
 * VARIABLE INFOS
 */

struct SymbolInfo;

typedef std::shared_ptr<SymbolInfo> SymbolInfoPtr;

enum VARIABLE_TYPE { VAR_LOCAL, VAR_FIELD, VAR_THIS };

/**
 * @brief Base class for VariableInfos
 */
struct VariableInfo : public common::MemoryManageable {
    virtual ~VariableInfo() {}

    virtual VariableInfo* asCaptured(CompCtx_Ptr ctx) = 0;
    virtual VARIABLE_TYPE type() const = 0;

    bool isMutable() const {
        return _isMutable;
    }
    bool isCaptured() const {
        return _isCaptured;
    }
    void setMutable() {
        _isMutable = true;
    }
    void setCaptured() {
        _isCaptured = true;
    }

    SymbolInfoPtr symInfo;

protected:
    VariableInfo() : _isMutable(false), _isCaptured(false) {}
    bool _isMutable;
    bool _isCaptured;
};

/**
 * @brief Represents a local variable
 */
struct LocalInfo final : public VariableInfo {
    LocalInfo() {}
    virtual ~LocalInfo() {}

    static LocalInfo* from(VariableInfo* info) {
        return (info && info->type() == VAR_LOCAL) ? static_cast<LocalInfo*>(info) : nullptr;
    }

    VariableInfo* asCaptured(CompCtx_Ptr) override {
        return this;
    }

    VARIABLE_TYPE type() const override {
        return VAR_LOCAL;
    }
};

/**
 * @brief Represents a field of a class
 */
struct FieldInfo final : public VariableInfo {
    FieldInfo(sym::VariableSymbol* thisClassSymbol) : thisClassSymbol(thisClassSymbol) {}

    virtual ~FieldInfo() {}

    static FieldInfo* from(VariableInfo* info) {
        return (info && info->type() == VAR_FIELD) ? static_cast<FieldInfo*>(info) : nullptr;
    }

    VariableInfo* asCaptured(CompCtx_Ptr) override {
        return this;
    }

    VARIABLE_TYPE type() const override {
        return VAR_FIELD;
    }

    sym::VariableSymbol* thisClassSymbol;
};

struct ThisInfo final : public VariableInfo {
    ThisInfo() {}
    virtual ~ThisInfo() {}

    VariableInfo* asCaptured(CompCtx_Ptr ctx) override {
        return ctx->memoryManager().New<LocalInfo>();
    }

    VARIABLE_TYPE type() const override {
        return VAR_THIS;
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
        DefineDecl* initDef = _mngr.New<DefineDecl>(initIdent, nullptr, func, DefFlags::NONE);

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

// CHANGE

Change::Change(Identifier* nf, Identifier* ia) : newField(nf), initializerArg(ia) {}

sym::VariableSymbol* Change::getNewFieldSymbol() const {
    return sym::getIfSymbolOfType<sym::VariableSymbol>(newField->getSymbol());
}

// CLASS PATH

ClassPatch::ClassPatch(Identifier* initializer, const std::vector<Change>& changes, const std::map<Identifier*, sym::Symbol*>& fieldCaptures)
    : _initalizer(initializer), _changes(changes), _fieldCaptures(fieldCaptures) {}

ClassPatch::~ClassPatch() {}

Identifier* ClassPatch::getInitializer() const {
    return _initalizer;
}

const std::vector<Change>& ClassPatch::getChanges() const {
    return _changes;
}

const std::map<Identifier*, sym::Symbol*>& ClassPatch::getFieldCaptures() const {
    return _fieldCaptures;
}

// CLASS PATCHER

struct ClassPatcher final {
    ClassPatcher(ClassDecl* clss, CompCtx_Ptr& ctx)
        : _clss(clss), _initBuilder(clss, ctx), _ctx(ctx), _mngr(_ctx->memoryManager()) {}

    void addFieldCapture(Identifier* id) {
        _fieldCaptures[id] = id->getSymbol();
    }

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
        return _mngr.New<ClassPatch>(_initBuilder.build(), _changes, _fieldCaptures);
    }

private:

    ClassDecl* _clss;
    InitializerBuilder _initBuilder;
    std::vector<Change> _changes;
    std::map<Identifier*, sym::Symbol*> _fieldCaptures;

    CompCtx_Ptr _ctx;
    common::AbstractMemoryManager& _mngr;
};

// PRE-TRANSFORM ANALYSIS

PreTransformAnalysis::PreTransformAnalysis(CompCtx_Ptr& ctx) : ASTImplicitVisitor(ctx) {

}

PreTransformAnalysis::~PreTransformAnalysis() {

}

void PreTransformAnalysis::visitClassDecl(ClassDecl* clss) {
    if (clss->getParent()) {
        clss->getParent()->onVisit(this);
    }
    for (TypeDecl* tdecl: clss->getTypeDecls()) {
        tdecl->getExpression()->onVisit(this);
    }
    for (TypeSpecifier* field : clss->getFields()) {
        field->getTypeNode()->onVisit(this);
    }
    for (DefineDecl* decl : clss->getDefs()) {
        if (TypeExpression* expr = decl->getTypeSpecifier()) {
            expr->onVisit(this);
        }
        if (Expression* val = decl->getValue()) {
            val->onVisit(this);
        }
    }
}

ClassDecl* PreTransformAnalysis::getParentMostClass(ClassDecl* clss) {
    ClassDecl* parentMost = clss;

    while (TypeExpression* parent = parentMost->getParent()) {
        if (type::Type* tp = ASTTypeCreator::createType(parent, _ctx)) {
            if (type::ProperType* pt = type::getIf<type::ProperType>(tp->applyTCCallsOnly(_ctx))) {
                parentMost = pt->getClass();
                continue;
            }
        }
        break;
    }

    return parentMost;
}

void PreTransformAnalysis::visit(ClassDecl* clss) {
    SAVE_MEMBER_AND_SET(_usedVars, {})
    SAVE_MEMBER_AND_SET(_boundVars, {})

    ClassDecl* parentMost = getParentMostClass(clss);
    sym::VariableSymbol*& thisClassSymbol = _classThisSymbols[parentMost];

    if (!thisClassSymbol) {
        // Create the `this` class symbol
        thisClassSymbol = _mngr.New<sym::VariableSymbol>(parentMost->getName() + ".this", "");
        setVariableInfo(thisClassSymbol, _mngr.New<ThisInfo>());
    }

    // Assign the FieldInfo to every field of the class
    for (const auto& pair : clss->getScope()->getAllSymbols()) {
        if (sym::Symbol* s = pair.second.symbol) {
            if (!getVariableInfo(s)) {
                // If not a static member, assign it a FieldInfo
                if (!(sym::getIfSymbolOfType<sym::TypeSymbol>(s) ||
                       (sym::getIfSymbolOfType<sym::DefinitionSymbol>(s) &&
                        sym::getIfSymbolOfType<sym::DefinitionSymbol>(s)->getDef()->isStatic())
                    )) {
                    setVariableInfo(s, _mngr.New<FieldInfo>(thisClassSymbol));
                }
            }
        }
    }

    // Visit the trees
    visitClassDecl(clss);

    // Drop the vars that were bound during the visit, as they are not free
    // and must therefore not be captured
    for (sym::VariableSymbol* var : _boundVars) {
        _usedVars.erase(var);
    }
    _usedVars.erase(thisClassSymbol);

    ClassPatcher classPatcher(clss, _ctx);

    // For all the captures
    for (const auto& freeVar : _usedVars) {
        sym::VariableSymbol* capturedSymbol(freeVar.first);
        const std::vector<Identifier*>& referringCapturedSymbol(freeVar.second);

        VariableInfo* capturedInfo = getVariableInfo(capturedSymbol);
        capturedInfo->setCaptured();

        // Add a new field to the class, which has the name of the captured variable.
        Change change = classPatcher.addNewField(capturedSymbol->getName());
        // Make that new field has the same variable infos
        setVariableInfo(change.getNewFieldSymbol(), capturedInfo->asCaptured(_ctx));

        // If `this` is captured, things work a bit differently:
        // Not all the Identifiers referring to this symbol really do yet, they may
        // have been added to the `referringCapturedSymbol` vector as a hint that they
        // are class field captures. To deal with class field captures, find among the
        // identifiers those which have a FieldInfo and add them to the ClassPatch's field captures.
        if (getVariableInfo(capturedSymbol)->type() == VAR_THIS) {
            for (Identifier* ident : referringCapturedSymbol) {
                if (getVariableInfo(ident->getSymbol())->type() == VAR_FIELD) {
                    classPatcher.addFieldCapture(ident);
                }
            }
        }
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
            info = setVariableInfo(ident->getSymbol(), _mngr.New<LocalInfo>());
            if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(ident->getSymbol())) {
                if (var->hasProperty(UsageProperty::MUTABLE)) {
                    info->setMutable();
                }
            }
        }

        if (info) {
            if (FieldInfo* field = FieldInfo::from(info)) {
                _usedVars[field->thisClassSymbol].push_back(ident);
            } else if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(ident->getSymbol())) {
                _usedVars[var].push_back(ident);
            }
        }
    }
}

// PRE-TRANSFORM IMPLEMENTATION

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

    SAVE_MEMBER_AND_SET(_curCapturedFields, classPatch->getFieldCaptures())

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

    RESTORE_MEMBER(_curCapturedFields)
}

void PreTransformImplementation::visit(DefineDecl* decl) {
    update(decl, decl->getName(),
           transform<TypeExpression>(decl->getTypeSpecifier()),
           transform<Expression>(decl->getValue()),
           decl->getFlags());
}

void PreTransformImplementation::visit(MemberAccess* dot) {
    update(dot,
           transform<Expression>(dot->getAccessed()),
           dot->getMember());
}

void PreTransformImplementation::visit(FunctionCreation* func) {
    if (type::ProperType* pt = type::getIf<type::ProperType>(func->type())) {
        Instantiation* inst = make<Instantiation>(pt->getClass());
        inst->setType(ASTTypeCreator::createType(inst->getInstantiatedExpression(), _ctx));

        transform<Expression>(inst);
    } else {
        Expression* newBody = transformFuncBody(func->getBody(), func->getArgs());

        update(func, func->getName(),
               transform<TypeTuple>(func->getTypeArgs()),
               transform<Expression>(func->getArgs()),
               newBody,
               transform<TypeExpression>(func->getReturnType()));
    }
}

void PreTransformImplementation::visit(TypeSpecifier* tps) {
    update(tps, tps->getSpecified(), transform<TypeExpression>(tps->getTypeNode()));

    if (isCapturedLocalMutableVar(tps->getSpecified()->getSymbol())) {
        makeAccessToBoxedValueOf(make<AssignmentExpression>(tps, makeBoxInstantiationOf(tps->type())));
    }
}

void PreTransformImplementation::visit(Instantiation* inst) {
    ASTTransformer::visit(inst);

    TypeExpression* instantiatedExpr = inst->getInstantiatedExpression();
    Expression* res = inst;

    while (true) {
        if (type::Type* tp = ASTTypeCreator::createType(instantiatedExpr, _ctx)) {
            if (type::ProperType* pt = type::getIf<type::ProperType>(tp->applyTCCallsOnly(_ctx))) {
                ClassPatch* classPatch = pt->getClass()->getUserdata<ClassPatch>();

                if (Identifier* initializer = classPatch->getInitializer()) {
                    MemberAccess* dot = make<MemberAccess>(res, initializer);
                    dot->setSymbol(initializer->getSymbol());
                    dot->setType(initializer->type());

                    std::vector<Expression*> args;
                    for (Change change : classPatch->getChanges()) {
                        if (isCapturedLocalMutableVar(change.initializerArg->getSymbol())) {
                            // If the capture was a local mutable variable, we want to send the
                            // boxed value, therefore we don't take the transform<Expression> path.
                            args.push_back(change.initializerArg);
                        } else {
                            args.push_back(transform<Expression>(change.initializerArg));
                        }
                    }

                    FunctionCall* call = make<FunctionCall>(dot, nullptr, make<Tuple>(args));
                    call->setType(inst->type());
                    res = call;
                }

                if ((instantiatedExpr = pt->getClass()->getParent())) {
                    continue;
                }
            }
        }

        break;
    }

    set(res);
}

void PreTransformImplementation::visit(Identifier* ident) {
    ASTTransformer::visit(ident);

    if (isCapturedClassField(ident)) {
        makeAccessToCapturedClassField(ident);
    } else if (isCapturedLocalMutableVar(ident->getSymbol())) {
        makeAccessToBoxedValueOf(ident);
    } else if (isClassThis(ident->getSymbol())) {
        makeAccessToClassThis();
    }
}

type::ProperType* PreTransformImplementation::boxOf(type::Type* tp) {
    type::ConstructorApplyType apply(_boxType, {tp});
    return type::getIf<type::ProperType>(apply.apply(_ctx));
}

bool PreTransformImplementation::isCapturedClassField(Identifier* ident) const {
    return _curCapturedFields.find(ident) != _curCapturedFields.end();
}

bool PreTransformImplementation::isCapturedLocalMutableVar(sym::Symbol* symbol) const {
    if (symbol) {
        if (VariableInfo* info = getVariableInfo(symbol)) {
            return info->type() == VAR_LOCAL && info->isMutable() && info->isCaptured();
        }
    }
    return false;
}

bool PreTransformImplementation::isClassThis(sym::Symbol* symbol) const {
    if (symbol) {
        if (VariableInfo* info = getVariableInfo(symbol)) {
            return info->type() == VAR_THIS;
        }
    }
    return false;
}

Expression* PreTransformImplementation::makeBoxInstantiationOf(type::Type* tp) {
    type::ProperType* boxOfT = boxOf(tp);
    TypeIdentifier* tid = make<TypeIdentifier>("Box");
    tid->setSymbol(_boxSymbol);
    Instantiation* inst = make<Instantiation>(tid);
    inst->setType(boxOfT);
    return inst;
}

Expression* PreTransformImplementation::makeAccessToCapturedClassField(Identifier* ident) {
    sym::Symbol* s = _curCapturedFields[ident];
    Identifier* member = _mngr.New<Identifier>(s->getName());
    member->setSymbol(s);

    Identifier* thisClass = _mngr.New<Identifier>(ident->getSymbol()->getName());
    thisClass->setSymbol(ident->getSymbol());

    MemberAccess* dot = make<MemberAccess>(thisClass, member);
    dot->setSymbol(s);

    if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(s)) {
        dot->setType(var->type());
    } else if (sym::DefinitionSymbol* def = sym::getIfSymbolOfType<sym::DefinitionSymbol>(s)) {
        dot->setType(def->type());
    }

    return dot;
}

Expression* PreTransformImplementation::makeAccessToBoxedValueOf(Expression* expr) {
    MemberAccess* dot = make<MemberAccess>(expr, _boxValueFieldIdent);
    dot->setSymbol(_boxValueFieldSym);
    dot->setType(expr->type());
    return dot;
}

This* PreTransformImplementation::makeAccessToClassThis() {
    return make<This>();
}

Expression* PreTransformImplementation::transformFuncBody(Expression* oldBody, Expression* args) {
    std::vector<Expression*> exprs;

    for (sym::VariableSymbol* param : ASTAssignmentChecker::getAssignedVars(args, _ctx)) {
        if (isCapturedLocalMutableVar(param)) {
            Identifier* paramIdent = make<Identifier>(param->getName());
            paramIdent->setSymbol(param);
            exprs.push_back(make<AssignmentExpression>(
                                makeAccessToBoxedValueOf(make<AssignmentExpression>(
                                                             paramIdent,
                                                             makeBoxInstantiationOf(param->type()))),
                                paramIdent));
        }
    }

    exprs.push_back(transform<Expression>(oldBody));
    return exprs.size() > 1 ? make<Block>(exprs) : exprs[0];
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

    bool isVisible = visibilityFromAnnotable(tdecl) || tdecl->isExtern();
    tsym->setUserdata(_mngr.New<DefUserData>(nameFromSymbol(tsym, isVisible), isVisible));
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
            if (localDecls[i]->isStatic()) {
                continue;
            }

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

    if (decl->isConstructor()) {
        _nextConstructorExpr = decl->getValue();
    }

    ASTImplicitVisitor::visit(decl);
    sym::DefinitionSymbol* def = decl->getSymbol();

    bool isVisible = visibilityFromAnnotable(decl) || decl->isExtern();
    bool success;

    if (def->type()->getTypeKind() == type::TYPE_METHOD) {
        def->setUserdata(_mngr.New<VirtualDefUserData>(nameFromDefSymbol(def, isVisible, success), isVisible));
    } else {
        bool isEntryPoint = false;
        decl->matchAnnotation<>("entry", [&](){ isEntryPoint = true; });

        std::string name = isEntryPoint ? "$ENTRY_POINT$" : nameFromDefSymbol(def, isVisible, success);

        def->setUserdata(_mngr.New<DefUserData>(name, isVisible || isEntryPoint));
    }

    if (decl->getTypeSpecifier()) {
        if (isVisible && !success) {
            _ctx->reporter().error(*decl->getTypeSpecifier(), "Type specifier contains type declarations that cannot be imported or exported");
        }
    } else if (isVisible) {
        _ctx->reporter().error(*decl, "Imported or exported definitions must have their type manually specified");
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

std::string UserDataAssignment::nameFromSymbol(sym::Symbol* s, bool isVisible) {
    const std::string& name = s->getAbsoluteName();

    if (!isVisible) {
        return freshName(name);
    } else {
        return name;
    }
}

std::string UserDataAssignment::nameFromDefSymbol(sym::DefinitionSymbol* s, bool isVisible, bool& signaturePrinterSuccess) {
    const std::string& name = s->getAbsoluteName();

    if (!isVisible) {
        return freshName(name);
    } else if (TypeExpression* tpe = s->getDef()->getTypeSpecifier()) {
        return name + ":" + ASTSignaturePrinter::getSignatureOf(tpe, signaturePrinterSuccess, _ctx);
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

ClassUserData::ClassUserData(const std::string& defId, bool isHidden,
                             const std::vector<sym::VariableSymbol*>& fields,
                             const std::vector<sym::DefinitionSymbol*>& defs, bool isAbstract)
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
