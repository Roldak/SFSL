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

ASTPrinter::ASTPrinter(std::shared_ptr<common::CompilationContext> &ctx) : ASTVisitor(ctx) {

}

void ASTPrinter::visit(ModuleDecl *module) {
    std::cout << "module " << module->getName()->getValue() << " {" << std::endl;

    for (ASTNode* decl : module->getDeclarations()) {
        decl->onVisit(this);
    }

    std::cout << std::endl << "}" << std::endl;
}

void ASTPrinter::visit(DefineDecl *decl) {
    std::cout << "def ";

    decl->getName()->onVisit(this);

    std::cout << " = ";

    decl->getValue()->onVisit(this);

    std::cout << std::endl;
}


void ASTPrinter::visit(Identifier *ident) {
    std::cout << ident->getValue();
}

void ASTPrinter::visit(IntLitteral *intlit) {
    std::cout << intlit->getValue();
}

}

}
