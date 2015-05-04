//
//  ASTPrinter.cpp
//  SFSL
//
//  Created by Romain Beguet on 17.11.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ASTPrinter.h"

namespace sfsl {

namespace ast {

ASTPrinter::ASTPrinter(CompCtx_Ptr &ctx) : ASTVisitor(ctx), _indentCount(0) {

}

ASTPrinter::~ASTPrinter() {

}

void ASTPrinter::visit(ModuleDecl *module) {
    std::cout << "module " << module->getName()->getValue() << " {" << std::endl;

    ++_indentCount;

    for (ModuleDecl* mod : module->getSubModules()) {
        printIndents();
        mod->onVisit(this);
        std::cout << std::endl;
    }

    for (TypeDecl* type : module->getTypes()) {
        printIndents();
        type->onVisit(this);
        std::cout << std::endl;
    }

    for (DefineDecl* decl : module->getDeclarations()) {
        printIndents();
        decl->onVisit(this);
        std::cout << std::endl;
    }

    --_indentCount;

    printIndents();
    std::cout << "}" << std::endl;
}

void ASTPrinter::visit(TypeDecl* tdecl) {
    std::cout << "type ";

    tdecl->getName()->onVisit(this);

    std::cout << " = ";

    tdecl->getExpression()->onVisit(this);
}

void ASTPrinter::visit(ClassDecl* clss) {
    std::cout << "class " << clss->getName();

    if (clss->getParent()) {
        clss->getParent()->onVisit(this);
    }

    std::cout << " {" << std::endl;

    ++_indentCount;

    for (TypeSpecifier* field : clss->getFields()) {
        printIndents();
        Identifier* fieldName = static_cast<Identifier*>(field->getSpecified());
        std::cout << fieldName->getValue() << " : ";
        field->getTypeNode()->onVisit(this);
        std::cout << ";" << std::endl;
    }

    for (DefineDecl* decl : clss->getDefs()) {
        printIndents();
        decl->onVisit(this);
        std::cout << std::endl;
    }

    --_indentCount;

    printIndents();
    std::cout << "}";
}

void ASTPrinter::visit(DefineDecl* decl) {

    std::cout << "def ";

    decl->getName()->onVisit(this);

    std::cout << " = ";

    decl->getValue()->onVisit(this);
}

void ASTPrinter::visit(TypeTuple* ttuple) {
    std::cout << "[";

    const std::vector<Expression*>& args(ttuple->getExpressions());

    for (size_t i = 0, e = args.size(); i < e; ++i) {
        args[i]->onVisit(this);

        if (i != e - 1) {
            std::cout << ", ";
        }
    }

    std::cout << "]";
}

void ASTPrinter::visit(TypeConstructorCreation* typeconstructor) {
    std::cout << "(";
    typeconstructor->getArgs()->onVisit(this);
    std::cout << " => ";
    typeconstructor->getBody()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(ExpressionStatement* exp) {
    exp->getExpression()->onVisit(this);
    std::cout << ";";
}

void ASTPrinter::visit(BinaryExpression* exp) {
    std::cout << "(";
    exp->getLhs()->onVisit(this);
    std::cout << " ";
    exp->getOperator()->onVisit(this);
    std::cout << " ";
    exp->getRhs()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(AssignmentExpression* aex) {
    std::cout << "(";
    aex->getLhs()->onVisit(this);
    std::cout << " = ";
    aex->getRhs()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(TypeSpecifier* tps) {
    std::cout << "(";
    tps->getSpecified()->onVisit(this);
    std::cout << " : ";
    tps->getTypeNode()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(Block* block) {
    std::cout << "{" << std::endl;
    ++_indentCount;

    for (auto stat : block->getStatements()) {
        printIndents();
        stat->onVisit(this);
        std::cout << std::endl;
    }

    --_indentCount;
    printIndents();
    std::cout << "}";
}

void ASTPrinter::visit(IfExpression* ifexpr) {
    std::cout << "if ";

    ifexpr->getCondition()->onVisit(this);

    std::cout << " ";

    ifexpr->getThen()->onVisit(this);

    if (ifexpr->getElse()) {
        std::cout << " else ";
        ifexpr->getElse()->onVisit(this);
    }
}

void ASTPrinter::visit(MemberAccess* dot) {
    std::cout << "(";
    dot->getAccessed()->onVisit(this);
    std::cout << ".";
    dot->getMember()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(Tuple* tuple) {
    std::cout << "(";

    const std::vector<Expression*>& args(tuple->getExpressions());

    for (size_t i = 0, e = args.size(); i < e; ++i) {
        args[i]->onVisit(this);

        if (i != e - 1) {
            std::cout << ", ";
        }
    }

    std::cout << ")";
}

void ASTPrinter::visit(FunctionCreation* func) {
    std::cout << "(";
    func->getArgs()->onVisit(this);
    std::cout << " => ";
    func->getBody()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(Identifier* ident) {
    std::cout << ident->getValue();
}

void ASTPrinter::visit(IntLitteral* intlit) {
    std::cout << intlit->getValue();
}

void ASTPrinter::visit(RealLitteral* reallit) {
    std::cout << reallit->getValue();
}

void ASTPrinter::printIndents() {
    for (size_t i = 0; i < _indentCount; ++i) {
        std::cout << "    ";
    }
}

}

}
