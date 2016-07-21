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

class PrintType : public Phase {
public:
    PrintType() : Phase("PrintType", "Handles 'printType' annotations") {}
    virtual ~PrintType() {}

    virtual std::vector<std::string> runsAfter() const override { return {"TypeChecking"}; }
    virtual std::vector<std::string> runsBefore() const override { return {"PreTransform"}; }

    virtual bool run(PhaseContext& pctx) override {
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");
        ast::Program* prog = pctx.require<ast::Program>("prog");

        class PrintTypeVisitor : public ast::ASTImplicitVisitor {
        public:
            PrintTypeVisitor(CompCtx_Ptr ctx) : ast::ASTImplicitVisitor(ctx) {}
            virtual ~PrintTypeVisitor() {}

            virtual void visit(ast::DefineDecl* decl) override {
                decl->matchAnnotation<>("printType", [=](){
                    _ctx->reporter().info(*decl, decl->getSymbol()->type()->toString());
                });
				
				ast::ASTImplicitVisitor::visit(decl);
            }

        } printTypeVisitor(ctx);

        prog->onVisit(&printTypeVisitor);

        return true;
    }
};

class FunctionCallNotification : public Phase {
public:
    FunctionCallNotification() : Phase("FunctionCallNotification", "Notifies when a function is called") {}
    virtual ~FunctionCallNotification() {}

    virtual std::string runsRightBefore() const override { return "NameAnalysis"; }

    virtual bool run(PhaseContext& pctx) override {
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");
        ast::Program* prog = pctx.require<ast::Program>("prog");

        class FunctionCallNotifier : public ast::ASTTransformer {
        public:
            FunctionCallNotifier(CompCtx_Ptr ctx) : ast::ASTTransformer(ctx) {}
            virtual ~FunctionCallNotifier() {}

            virtual void visit(ast::FunctionCall* call) override {
                std::vector<ast::Expression*> args {make<ast::StringLiteral>("FUNCTION CALL")};
                std::vector<ast::Expression*> stmts {
                    make<ast::FunctionCall>(make<ast::Identifier>("println"), nullptr, make<ast::Tuple>(args)),
                    update(call, transform<ast::Expression>(call->getCallee()), call->getTypeArgsTuple(), transform<ast::Tuple>(call->getArgsTuple()))
                };
                make<ast::Block>(stmts);
            }
        } notifier(ctx);

        prog = notifier.transform<ast::Program>(prog);

        return true;
    }
};