//
//  ASTTransformer.h
//  SFSL
//
//  Created by Romain Beguet on 20.07.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__ASTTransformer__
#define __SFSL__ASTTransformer__

#include "ASTVisitor.h"
#include "ASTTypeIdentifier.h"

namespace sfsl {

namespace ast {

class ASTTransformer : public ASTVisitor {
public:

    ASTTransformer(CompCtx_Ptr& ctx);

    virtual ~ASTTransformer();

    virtual void visit(ASTNode* node) override;

    virtual void visit(Program* prog) override;

    virtual void visit(ModuleDecl* module) override;
    virtual void visit(TypeDecl* tdecl) override;
    virtual void visit(ClassDecl* clss) override;
    virtual void visit(DefineDecl* decl) override;

    virtual void visit(ProperTypeKindSpecifier* ptks) override;
    virtual void visit(TypeConstructorKindSpecifier* tcks) override;

    virtual void visit(FunctionTypeDecl* ftdecl) override;
    virtual void visit(TypeMemberAccess* tdot) override;
    virtual void visit(TypeTuple* ttuple) override;
    virtual void visit(TypeConstructorCreation* typeconstructor) override;
    virtual void visit(TypeConstructorCall* tcall) override;
    virtual void visit(TypeIdentifier* tident) override;
    virtual void visit(TypeToBeInferred* tbi) override;
    virtual void visit(TypeParameter* tparam) override;

    virtual void visit(ExpressionStatement* exp) override;

    virtual void visit(AssignmentExpression* aex) override;
    virtual void visit(TypeSpecifier* tps) override;
    virtual void visit(Block* block) override;
    virtual void visit(IfExpression* ifexpr) override;
    virtual void visit(MemberAccess* dot) override;
    virtual void visit(Tuple* tuple) override;
    virtual void visit(FunctionCreation* func) override;
    virtual void visit(FunctionCall* call) override;
    virtual void visit(Instantiation* inst) override;
    virtual void visit(Identifier* ident) override;
    virtual void visit(This* ths) override;
    virtual void visit(BoolLiteral* boollit) override;
    virtual void visit(IntLiteral* intlit) override;
    virtual void visit(RealLiteral* reallit) override;
    virtual void visit(StringLiteral* strlit) override;

protected:

    template<typename T>
    T* transform(ASTNode* node, bool canReturnNull = false) {
        if (!node) {
            return nullptr;
        }

        node->onVisit(this);

        if (_created == nullptr) {
            if (!canReturnNull) {
                _ctx->reporter().fatal(*node, "AST transformer produced nothing though it was no allowed to");
            }
        } else if (!isNodeOfType<T>(static_cast<ASTNode*>(_created), _ctx)) {
            _ctx->reporter().fatal(*node, "AST transformer produced wrong kind of node");
        }

        return static_cast<T*>(_created);
    }

    template<typename T, typename K>
    std::vector<T*> transform(std::vector<K> oldNodes, bool canBeShrinked = true) {
        std::vector<T*> newNodes;
        for (ASTNode* oldNode : oldNodes) {
            if (T* newNode = transform<T>(oldNode, canBeShrinked)) {
                newNodes.push_back(newNode);
            }
        }
        return newNodes;
    }

    void set(void* node) {
        _created = node;
    }

    template<typename T, typename... Args>
    T* make(Args... args) {
        T* toRet = _mngr.New<T>(std::forward<Args>(args)...);
        _created = toRet;
        return toRet;
    }

#define SAVE_TRAIT(trait, node) \
    auto saved##trait = save##trait (node);

#define RESTORE_TRAIT(trait, node) \
    restore##trait (node, saved##trait);

    template<typename T, typename... Args>
    T* update(T* old, Args... args) {
        SAVE_TRAIT(Scoped, old)
        SAVE_TRAIT(CanUseModules, old)
        SAVE_TRAIT(Annotable, old)
        SAVE_TRAIT(TypeParametrizable, old)
        SAVE_TRAIT(UserData, old)
        SAVE_TRAIT(Typed, old)
        SAVE_TRAIT(Kinded, old)
        SAVE_TRAIT(Positionnable, old)
        SAVE_TRAIT(Symbolic, old)

        *old = T(std::forward<Args>(args)...);

        RESTORE_TRAIT(Scoped, old)
        RESTORE_TRAIT(CanUseModules, old)
        RESTORE_TRAIT(Annotable, old)
        RESTORE_TRAIT(TypeParametrizable, old)
        RESTORE_TRAIT(UserData, old)
        RESTORE_TRAIT(Typed, old)
        RESTORE_TRAIT(Kinded, old)
        RESTORE_TRAIT(Positionnable, old)
        RESTORE_TRAIT(Symbolic, old)

        _created = old;
        return old;
    }

#define IMPL_TRAIT_SAVER_RESTORER(trait, type) \
    inline void* save##trait(void* node) { return node; } \
    inline void restore##trait(void*, void*) { } \
    inline type save##trait(type* traited) { return *traited; } \
    inline void restore##trait(type* node, const type& savedTrait) { *node = savedTrait; }

#define IMPL_TEMPLATED_TRAIT_SAVER_RESTORER(trait, type) \
    inline void* save##trait(void* node) { return node; } \
    inline void restore##trait(void*, void*) { } \
    template<typename T> \
    inline type<T> save##trait(type<T>* traited) { return *traited; } \
    template<typename T> \
    inline void restore##trait(type<T>* node, const type<T>& savedTrait) { *node = savedTrait; }

    IMPL_TRAIT_SAVER_RESTORER(Scoped, sym::Scoped)
    IMPL_TRAIT_SAVER_RESTORER(CanUseModules, ast::CanUseModules)
    IMPL_TRAIT_SAVER_RESTORER(Annotable, ast::Annotable)
    IMPL_TRAIT_SAVER_RESTORER(TypeParametrizable, type::TypeParametrizable)
    IMPL_TRAIT_SAVER_RESTORER(UserData, common::HasManageableUserdata)
    IMPL_TRAIT_SAVER_RESTORER(Expression, ast::Expression)
    IMPL_TRAIT_SAVER_RESTORER(Typed, type::Typed)
    IMPL_TRAIT_SAVER_RESTORER(Kinded, kind::Kinded)
    IMPL_TRAIT_SAVER_RESTORER(Positionnable, common::Positionnable)
    IMPL_TEMPLATED_TRAIT_SAVER_RESTORER(Symbolic, sym::Symbolic)

    void* _created;
};

}

}

#endif
