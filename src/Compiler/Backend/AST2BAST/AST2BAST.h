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
    virtual void visit(ast::TypeParameter* tparam) override;

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
    virtual void visit(ast::BoolLiteral* boollit) override;
    virtual void visit(ast::IntLiteral* intlit) override;
    virtual void visit(ast::RealLiteral* reallit) override;
    virtual void visit(ast::StringLiteral* strlit) override;

    Program* transform(ast::Program* node);

private:

    template<typename T>
    void visitSymbolic(T* symbolic);

    void assignIdentifier(ast::Identifier* ident, ast::Expression* val);

    const std::string& getDefId(ast::FunctionCreation* func);
    const std::string& getDefId(ast::ClassDecl* clss);
    const std::string& getDefId(sym::DefinitionSymbol* def);
    const std::string& getDefId(sym::TypeSymbol* tpe);

    template<typename T>
    bool isVisibleDef(T* def) const;

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

    BASTSimplifier();
    virtual ~BASTSimplifier();

    virtual void visit(Program* prog) override;

private:

    class Analyser : public BASTExplicitVisitor {
    public:
        Analyser();
        virtual ~Analyser();

        virtual void visit(Program* prog) override;
        virtual void visit(GlobalDef* global) override;
        virtual void visit(DefIdentifier* defid) override;

        const std::map<std::string, std::string>& getHiddenToAnyMappings() const;
        const std::map<std::string, std::string>& getVisibleToHiddenMappings() const;

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

        HiddenToAnyRenamer(const std::map<std::string, std::string>& map);
        virtual ~HiddenToAnyRenamer();

        virtual void visit(Program* prog) override;
        virtual void visit(DefIdentifier* defid) override;

    private:

        const std::map<std::string, std::string>& _map;
    };

    class VisibleToHiddenRenamer : public BASTImplicitVisitor {
    public:

        VisibleToHiddenRenamer(const std::map<std::string, std::string>& map);
        virtual ~VisibleToHiddenRenamer();

        virtual void visit(Program* prog) override;

        virtual void visit(MethodDef* meth) override;
        virtual void visit(ClassDef* clss) override;
        virtual void visit(GlobalDef* global) override;

        virtual void visit(DefIdentifier* defid) override;

    private:

        bool _toDelete;
        BASTNode* _nextExpr;

        const std::map<std::string, std::string>& _map;
    };
};

}

}

#endif
