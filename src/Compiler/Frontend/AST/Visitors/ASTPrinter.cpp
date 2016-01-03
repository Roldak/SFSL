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

ASTPrinter::ASTPrinter(CompCtx_Ptr &ctx, std::ostream& ostream)
    : ASTVisitor(ctx), _indentCount(0), _ostream(ostream) {

}

ASTPrinter::~ASTPrinter() {

}

void ASTPrinter::visit(ASTNode*) {
    throw common::CompilationFatalError("Unimplemented visitor");
}

void ASTPrinter::visit(Program* prog) {
    for (ast::ModuleDecl* mod : prog->getModules()) {
        mod->onVisit(this);
    }
}

void ASTPrinter::visit(ModuleDecl* module) {
    _ostream << "module " << module->getName()->getValue() << " {" << std::endl;

    ++_indentCount;

    printUsings(*module, false);

    for (ModuleDecl* mod : module->getSubModules()) {
        printIndents();
        mod->onVisit(this);
        _ostream << std::endl;
    }

    for (TypeDecl* type : module->getTypes()) {
        printIndents();
        type->onVisit(this);
        _ostream << std::endl;
    }

    for (DefineDecl* decl : module->getDeclarations()) {
        printIndents();
        decl->onVisit(this);
        _ostream << std::endl;
    }

    --_indentCount;

    printIndents();
    _ostream << "}" << std::endl;
}

void ASTPrinter::visit(TypeDecl* tdecl) {
    _ostream << "type ";

    tdecl->getName()->onVisit(this);

    _ostream << " = ";

    tdecl->getExpression()->onVisit(this);
}

void ASTPrinter::visit(ClassDecl* clss) {
    _ostream << "class " << clss->getName();

    if (clss->getParent()) {
        _ostream << " : ";
        clss->getParent()->onVisit(this);
    }

    _ostream << " {" << std::endl;

    ++_indentCount;

    for (TypeDecl* tdecl : clss->getTypeDecls()) {
        printIndents();
        tdecl->onVisit(this);
        _ostream << std::endl;
    }

    for (TypeSpecifier* field : clss->getFields()) {
        printIndents();
        Identifier* fieldName = static_cast<Identifier*>(field->getSpecified());
        _ostream << fieldName->getValue() << " : ";
        field->getTypeNode()->onVisit(this);
        _ostream << ";" << std::endl;
    }

    for (DefineDecl* decl : clss->getDefs()) {
        printIndents();
        decl->onVisit(this);
        _ostream << std::endl;
    }

    --_indentCount;

    printIndents();
    _ostream << "}";
}

void ASTPrinter::visit(DefineDecl* decl) {
    if (decl->isExtern()) {
        _ostream << "extern ";
    } else if (decl->isAbstract()) {
        _ostream << "abstract ";
    }

    _ostream << "def ";

    decl->getName()->onVisit(this);

    if (TypeExpression* expr = decl->getTypeSpecifier()) {
        _ostream << ": ";
        expr->onVisit(this);
    }

    if (Expression* val = decl->getValue()) {
        _ostream << " = ";
        val->onVisit(this);
    }
}

void ASTPrinter::visit(ProperTypeKindSpecifier*) {
    _ostream << "*";
}

void ASTPrinter::visit(TypeConstructorKindSpecifier* tcks) {
    _ostream << "[";
    for (size_t i = 0; i < tcks->getArgs().size(); ++i) {
        tcks->getArgs()[i]->onVisit(this);
        if (i != tcks->getArgs().size() - 1) {
            _ostream << ", ";
        }
    }
    _ostream << "]->";
    tcks->getRet()->onVisit(this);
}

void ASTPrinter::visit(FunctionTypeDecl* ftdecl) {
    _ostream << "(";
    for (size_t i = 0; i < ftdecl->getArgTypes().size(); ++i) {
        ftdecl->getArgTypes()[i]->onVisit(this);
        if (i != ftdecl->getArgTypes().size() - 1) {
            _ostream << ", ";
        }
    }
    _ostream << ")->";
    ftdecl->getRetType()->onVisit(this);
}

void ASTPrinter::visit(TypeMemberAccess* tdot) {
    _ostream << "(";
    tdot->getAccessed()->onVisit(this);
    _ostream << ".";
    tdot->getMember()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(TypeTuple* ttuple) {
    _ostream << "[";

    const std::vector<TypeExpression*>& args(ttuple->getExpressions());

    for (size_t i = 0, e = args.size(); i < e; ++i) {
        args[i]->onVisit(this);

        if (i != e - 1) {
            _ostream << ", ";
        }
    }

    _ostream << "]";
}

void ASTPrinter::visit(TypeConstructorCreation* typeconstructor) {
    _ostream << "(";
    typeconstructor->getArgs()->onVisit(this);
    _ostream << " => ";
    typeconstructor->getBody()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(TypeConstructorCall* tcall) {
    tcall->getCallee()->onVisit(this);
    tcall->getArgsTuple()->onVisit(this);
}

void ASTPrinter::visit(TypeIdentifier* tident) {
    _ostream << tident->getValue();
}

void ASTPrinter::visit(TypeToBeInferred*) {

}

void ASTPrinter::visit(KindSpecifier* ks) {
    ks->getSpecified()->onVisit(this);
    _ostream << " : ";
    ks->getKindNode()->onVisit(this);
}

void ASTPrinter::visit(ExpressionStatement* exp) {
    exp->getExpression()->onVisit(this);
    _ostream << ";";
}

void ASTPrinter::visit(AssignmentExpression* aex) {
    _ostream << "(";
    aex->getLhs()->onVisit(this);
    _ostream << " = ";
    aex->getRhs()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(TypeSpecifier* tps) {
    _ostream << "(";
    tps->getSpecified()->onVisit(this);
    _ostream << " : ";
    tps->getTypeNode()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(Block* block) {
    _ostream << "{" << std::endl;
    ++_indentCount;

    for (auto stat : block->getStatements()) {
        printIndents();
        stat->onVisit(this);
        _ostream << std::endl;
    }

    --_indentCount;
    printIndents();
    _ostream << "}";
}

void ASTPrinter::visit(IfExpression* ifexpr) {
    _ostream << "if ";

    ifexpr->getCondition()->onVisit(this);

    _ostream << " ";

    ifexpr->getThen()->onVisit(this);

    if (ifexpr->getElse()) {
        _ostream << " else ";
        ifexpr->getElse()->onVisit(this);
    }
}

void ASTPrinter::visit(MemberAccess* dot) {
    _ostream << "(";
    dot->getAccessed()->onVisit(this);
    _ostream << ".";
    dot->getMember()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(Tuple* tuple) {
    _ostream << "(";

    const std::vector<Expression*>& args(tuple->getExpressions());

    for (size_t i = 0, e = args.size(); i < e; ++i) {
        args[i]->onVisit(this);

        if (i != e - 1) {
            _ostream << ", ";
        }
    }

    _ostream << ")";
}

void ASTPrinter::visit(FunctionCreation* func) {
    _ostream << "(";
    func->getArgs()->onVisit(this);

    if (TypeExpression* retType = func->getReturnType()) {
        _ostream << "->";
        retType->onVisit(this);
    }

    _ostream << " => ";
    func->getBody()->onVisit(this);
    _ostream << ")";
}

void ASTPrinter::visit(FunctionCall* call) {
    call->getCallee()->onVisit(this);
    if (call->getTypeArgsTuple()) {
        call->getTypeArgsTuple()->onVisit(this);
    }
    call->getArgsTuple()->onVisit(this);
}

void ASTPrinter::visit(Instantiation* inst) {
    inst->getInstantiatedExpression()->onVisit(this);
}

void ASTPrinter::visit(Identifier* ident) {
    _ostream << ident->getValue();
}

void ASTPrinter::visit(This*) {
    _ostream << "this";
}

void ASTPrinter::visit(BoolLitteral* boollit) {
    _ostream << (boollit->getValue() ? "true" : "false");
}

void ASTPrinter::visit(IntLitteral* intlit) {
    _ostream << intlit->getValue();
}

void ASTPrinter::visit(RealLitteral* reallit) {
    _ostream << reallit->getValue();
}

void ASTPrinter::visit(StringLitteral* strlit) {
    _ostream << "\"" << strlit->getValue() << "\"";
}

void ASTPrinter::printIndents() {
    for (size_t i = 0; i < _indentCount; ++i) {
        _ostream << "    ";
    }
}

void ASTPrinter::printUsings(const CanUseModules& canUseModules, bool asStatement) {
    for (const CanUseModules::ModulePath& path : canUseModules.getUsedModules()) {
        printIndents();
        _ostream << "using " << path.toString();
        if (asStatement) {
            _ostream << ";";
        }
        _ostream << std::endl;
    }
}

}

}
