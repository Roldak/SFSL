#include <iostream>
#include "sfsl.h"

#include "Compiler/Frontend/AST/Visitors/ASTImplicitVisitor.h"
#include "Compiler/Frontend/AST/Nodes/Expressions.h"
#include "Compiler/Frontend/AST/Visitors/ASTSymbolExtractor.h"
#include "Compiler/Frontend/AST/Visitors/ASTTypeIdentifier.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"

using namespace sfsl;

class DivByZero : public Phase {
public:
    DivByZero() : Phase("DivByZero", "Checks divisions by zero") {}
    virtual ~DivByZero() {}

    virtual std::string runsRightAfter() const override { return "NameAnalysis"; }

    virtual bool run(PhaseContext& pctx) override {
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");
        ast::Program* prog = pctx.require<ast::Program>("prog");

        class DivByZeroChecker : public ast::ASTImplicitVisitor {
        public:
            DivByZeroChecker(CompCtx_Ptr ctx) : ast::ASTImplicitVisitor(ctx) {}
            virtual ~DivByZeroChecker() {}

            virtual void visit(ast::FunctionCall* call) override {
                if (ast::ASTSymbolExtractor::extractSymbol(call->getCallee(), _ctx)->getAbsoluteName() == "program.div") {
                    if (call->getArgs().size() == 2) {
                        ast::Expression* arg = call->getArgs()[1];
                        if (ast::isNodeOfType<ast::IntLiteral>(arg, _ctx)) {
                            ast::IntLiteral* lit = static_cast<ast::IntLiteral*>(arg);
                            if (lit->getValue() == 0) {
                                _ctx->reporter().error(*arg, "Division by zero!");
                            }
                        }
                    }
                }

                ast::ASTImplicitVisitor::visit(call);
            }
        } checker(ctx);

        prog->onVisit(&checker);

        return true;
    }
};