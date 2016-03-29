//
//  AST2BAST.h
//  SFSL
//
//  Created by Romain Beguet on on 28.03.16.
//  Copyright (c) 2016 Romain Beguet. All rights reserved.
//

#ifndef __SFSL__AST2BAST__
#define __SFSL__AST2BAST__

#include <list>

#include "../../Frontend/AST/Visitors/ASTExplicitVisitor.h"
#include "../BAST/Visitors/BASTImplicitVisitor.h"
#include "../BAST/Visitors/BASTExplicitVisitor.h"
#include "../BAST/Nodes/Nodes.h"
#include "UserDataAssignment.h"

namespace sfsl {

namespace bast {

class AST2BAST : public ast::ASTExplicitVisitor {
public:

    AST2BAST(CompCtx_Ptr& ctx);

    virtual ~AST2BAST();

    virtual void visit(ast::ASTNode*) override;

    virtual void visit(ast::Program* prog) override;

    virtual void visit(ast::ModuleDecl* module) override;
    virtual void visit(ast::TypeDecl* tdecl) override;
    virtual void visit(ast::ClassDecl* clss) override;
    virtual void visit(ast::DefineDecl* decl) override;

    virtual void visit(ast::ProperTypeKindSpecifier* ptks) override;
    virtual void visit(ast::TypeConstructorKindSpecifier* tcks) override;

    virtual void visit(ast::FunctionTypeDecl* ftdecl) override;
    virtual void visit(ast::TypeMemberAccess* tdot) override;
    virtual void visit(ast::TypeTuple* ttuple) override;
    virtual void visit(ast::TypeConstructorCreation* typeconstructor) override;
    virtual void visit(ast::TypeConstructorCall* tcall) override;
    virtual void visit(ast::TypeIdentifier* tident) override;
    virtual void visit(ast::TypeToBeInferred* tbi) override;
    virtual void visit(ast::KindSpecifier* ks) override;

    virtual void visit(ast::ExpressionStatement* exp) override;

    virtual void visit(ast::AssignmentExpression* aex) override;
    virtual void visit(ast::TypeSpecifier* tps) override;
    virtual void visit(ast::Block* block) override;
    virtual void visit(ast::IfExpression* ifexpr) override;
    virtual void visit(ast::MemberAccess* dot) override;
    virtual void visit(ast::Tuple* tuple) override;
    virtual void visit(ast::FunctionCreation* func) override;
    virtual void visit(ast::FunctionCall* call) override;
    virtual void visit(ast::Instantiation* inst) override;
    virtual void visit(ast::Identifier* ident) override;
    virtual void visit(ast::This* ths) override;
    virtual void visit(ast::BoolLitteral* boollit) override;
    virtual void visit(ast::IntLitteral* intlit) override;
    virtual void visit(ast::RealLitteral* reallit) override;
    virtual void visit(ast::StringLitteral* strlit) override;

    Program* transform(ast::Program* node);

private:

    template<typename T>
    void visitSymbolic(T* symbolic);

    const std::string& getDefId(ast::FunctionCreation* func);
    const std::string& getDefId(ast::ClassDecl* clss);
    const std::string& getDefId(sym::DefinitionSymbol* def);
    const std::string& getDefId(sym::TypeSymbol* tpe);

    template<typename T>
    bool isHiddenDef(T* def) const;

    template<typename BAST_NODE, typename DEF, typename... BAST_ARGS_REST>
    void addDefinitionToProgram(DEF* def, BAST_ARGS_REST... args);

    size_t getVarLoc(sym::VariableSymbol* var);
    bool isVariableAttribute(sym::VariableSymbol* var);

    template<typename T, typename... Args>
    T* make(Args... args);

    Expression* makeUnit();
    BASTNode* makeBad();

    Expression* transform(ast::Expression* node);

    bool alreadyTransformed(ast::DefUserData* defUD);

    size_t _freshId;
    std::string freshName(const std::string& prefix);

    std::vector<Definition*> _visibleDefs;
    std::vector<Definition*> _hiddenDefs;

    common::AbstractReporter& _rep;
    BASTNode* _created;
};

class BASTSimplifier : public BASTImplicitVisitor {
public:

    BASTSimplifier(CompCtx_Ptr& ctx);
    virtual ~BASTSimplifier();

    virtual void visit(Program* prog) override;

private:

    class Analyser : public BASTExplicitVisitor {
    public:
        Analyser(CompCtx_Ptr& ctx);
        virtual ~Analyser();

        virtual void visit(Program* prog) override;
        virtual void visit(GlobalDef* global) override;
        virtual void visit(DefIdentifier* defid) override;

        const std::map<std::string, std::string>& getHiddenToAnyMappings() const;

    private:

        bool isHiddenName(const std::string& name) const;
        std::string findSubstitution(std::string name) const;

        bool _processingVisibleNames;

        const std::string* _name;

        std::set<std::string> _visibleNames;

        std::map<std::string, std::string> _visibleToHiddenNameMappings;
        std::map<std::string, std::string> _hiddenToAnyNameMappings;
    };

    class HiddenToAnyRenamer : public BASTImplicitVisitor {
    public:

        HiddenToAnyRenamer(CompCtx_Ptr& ctx, const std::map<std::string, std::string>& map);
        virtual ~HiddenToAnyRenamer();

        virtual void visit(Program* prog) override;
        virtual void visit(DefIdentifier* defid) override;

    private:

        const std::map<std::string, std::string>& _map;
    };
};

}

}

#endif
