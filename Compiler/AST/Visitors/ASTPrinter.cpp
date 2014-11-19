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

ASTPrinter::ASTPrinter(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx), _indentCount(0) {

}

void ASTPrinter::visit(ModuleDecl *module) {
    printIndents();
    std::cout << "module " << module->getName()->getValue() << " {" << std::endl;

    ++_indentCount;

    for (ASTNode* decl : module->getDeclarations()) {
        printIndents();
        decl->onVisit(this);
        std::cout << std::endl;
    }

    --_indentCount;

    printIndents();
    std::cout << "}" << std::endl;
}

void ASTPrinter::visit(DefineDecl *decl) {

    std::cout << "def ";

    decl->getName()->onVisit(this);

    std::cout << " = ";

    decl->getValue()->onVisit(this);
}

void ASTPrinter::visit(ExpressionStatement *exp) {
    exp->getExpression()->onVisit(this);
    std::cout << ";";
}

void ASTPrinter::visit(BinaryExpression *exp) {
    std::cout << "(";
    exp->getLhs()->onVisit(this);
    std::cout << " ";
    exp->getOperator()->onVisit(this);
    std::cout << " ";
    exp->getRhs()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(Block *block) {
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

void ASTPrinter::visit(IfExpression *ifexpr) {
    std::cout << "if (";

    ifexpr->getCondition()->onVisit(this);

    std::cout << ") ";

    ifexpr->getThen()->onVisit(this);

    if (ifexpr->getElse()) {
        std::cout << " else ";
        ifexpr->getElse()->onVisit(this);
    }
}

void ASTPrinter::visit(MemberAccess *dot) {
    std::cout << "(";
    dot->getAccessed()->onVisit(this);
    std::cout << ".";
    dot->getMember()->onVisit(this);
    std::cout << ")";
}

void ASTPrinter::visit(Tuple *tuple) {
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

void ASTPrinter::visit(FunctionCreation *func) {
    func->getArgs()->onVisit(this);
    std::cout << " => ";
    func->getBody()->onVisit(this);
}

void ASTPrinter::visit(Identifier *ident) {
    std::cout << ident->getValue();
}

void ASTPrinter::visit(IntLitteral *intlit) {
    std::cout << intlit->getValue();
}

void ASTPrinter::visit(RealLitteral *reallit) {
    std::cout << reallit->getValue();
}

void ASTPrinter::printIndents() {
    for (size_t i = 0; i < _indentCount; ++i) {
        std::cout << "    ";
    }
}

}

}
