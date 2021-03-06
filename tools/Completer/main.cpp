#include <fstream>
#include "Compiler/Frontend/AST/Nodes/Program.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/Symbols/Scope.h"
#include "Visitors/ScopeIdentifier.h"
#include "ScopeReporter.h"
#include "sfsl.h"
#include "unistd.h"

#define K_ATTRIBUTE_ID  ('a')
#define K_METHOD_ID     ('m')

using namespace sfsl;

class ScopeIdentiferPhase : public Phase {
public:
    ScopeIdentiferPhase()
        : Phase("ScopeIdentifier", "Assigns scopes unique ids") { }
    virtual ~ScopeIdentiferPhase() { }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");

        complete::ScopeReporter id(ctx, complete::StandartScopeReporter::CoutReporter);

        prog->onVisit(&id);

        return true;
    }
};

class ScopeFinderPhase : public Phase {
public:
    ScopeFinderPhase(const std::string& toComplete, const std::vector<size_t>& scopeId)
        :   Phase("ScopeFinderPhase", "Only keeps scope that are parents of the given scope"),
            _toComplete(toComplete), _exprToComplete(nullptr), _scopeId(scopeId) {
    }

    virtual ~ScopeFinderPhase() { }

    virtual std::vector<std::string> runsBefore() const {
        return {"NameAnalysis"};
    }

    virtual bool run(PhaseContext& pctx) {
        ast::Program* prog = pctx.require<ast::Program>("prog");
        CompCtx_Ptr ctx = *pctx.require<CompCtx_Ptr>("ctx");
        common::AbstractPrimitiveNamer* namer = pctx.require<common::AbstractPrimitiveNamer>("namer");

        complete::ScopeFinder is(ctx, _scopeId);
        prog->onVisit(&is);

        ast::Block* blk = is.getBlock();

        if (!blk) {
            return false;
        }

        src::StringSource source(src::InputSourceName::make(ctx, "tmp"), _toComplete);
        lex::Lexer toCompleteLexer(ctx->memoryManager(), ctx->reporter(), source);
        ast::Parser toCompleteParser(ctx, toCompleteLexer, namer);

        _exprToComplete = toCompleteParser.parseSingleExpression();

        if (ctx->reporter().getErrorCount() > 0) {
            return false;
        }

        std::vector<ast::Expression*> exprs(blk->getStatements());
        exprs.push_back(_exprToComplete);

        *blk = ast::Block(exprs);

        return true;
    }

    ast::Expression* expressionToComplete() {
        return _exprToComplete;
    }

private:

    std::string _toComplete;
    ast::Expression* _exprToComplete;
    std::vector<size_t> _scopeId;
};

class CtxCollector : public AbstractOutputCollector {
public:
    CtxCollector() {}
    virtual ~CtxCollector() {}

    virtual void collect(PhaseContext& pctx) override {
        _ctx = *pctx.require<CompCtx_Ptr>("ctx");
    }

    CompCtx_Ptr getCtx() {
        return _ctx;
    }

private:

    CompCtx_Ptr _ctx;
};

class Completer final {
public:
    enum CompletionType {T_DOT, T_PARENTHESIS};

    Completer(CompCtx_Ptr ctx) : _ctx(ctx) {

    }

    ~Completer() { }

    void outputPossibilities(type::Type* tp, CompletionType ct) {
        if (ct == T_DOT) {
            if (type::ProperType* pt = type::getIf<type::ProperType>(tp->applyTCCallsOnly(_ctx))) {
                if (ast::ClassDecl* clss = pt->getClass()) {
                    sym::Scope* clssScope = clss->getScope();

                    for (const std::pair<std::string, sym::SymbolData>& entry : clssScope->getAllSymbols()) {
                        outputFromSymbolData(entry.second, tp->applyTCCallsOnly(_ctx)->getEnvironment());
                    }
                }
            }
        }
    }

private:

    void outputFromSymbolData(const sym::SymbolData& data, const type::Environment& env) {
        type::Type* symType = nullptr;
        char kindOfSym;

        if (sym::VariableSymbol* var = sym::getIfSymbolOfType<sym::VariableSymbol>(data.symbol)) {
            symType = var->type();
            kindOfSym = K_ATTRIBUTE_ID;
        } else if (sym::DefinitionSymbol* def = sym::getIfSymbolOfType<sym::DefinitionSymbol>(data.symbol)) {
            if (!def->getDef()->isRedef() && !def->getDef()->isConstructor()) {
                symType = def->type();
                kindOfSym = K_METHOD_ID;
            }
        }

        if (!symType) {
            return;
        }

        symType = symType->substitute(data.env, _ctx)->substitute(env, _ctx);

        std::cout << kindOfSym << ":" << data.symbol->getName() << ":" << symType->toString() << std::endl;
    }

    CompCtx_Ptr _ctx;
};

std::vector<size_t> scopeIdFromStr(const std::string& scope) {
    std::vector<std::string> split;
    std::vector<size_t> scopeId;

    utils::split(split, scope, '.');

    for (const std::string& str : split) {
        scopeId.push_back(utils::String_toT<size_t>(str));
    }
    return scopeId;
}

int main(int argc, char** argv) {
    // LOAD FILE

    char* sourceFile = NULL;
    char* scopeIdStr = NULL;
    char* exprToComplete = NULL;
    Completer::CompletionType completionType = Completer::T_DOT;
    bool generate = false;
    int option;

    while((option = getopt(argc, argv, "s:c:gt:")) != -1){
        switch (option) {
        case 's':
            scopeIdStr = optarg;
            break;
        case 'c':
            exprToComplete = optarg;
            break;
        case 'g':
            generate = true;
            break;
        case 't':
            switch(optarg[0]) {
            case 'd':
                completionType = Completer::T_DOT;
                break;
            case 'p':
                completionType = Completer::T_PARENTHESIS;
                break;
            default:
                break;
            }

            break;
        default:
            std::cerr << "unexpected program argument : " << option << std::endl;
        }
    }

    if (optind < argc) {
        sourceFile = argv[optind++];
    } else {
        std::cerr << "missing source file" << std::endl;
        return 1;
    }

    if (!exprToComplete && !generate) {
        std::cerr << "missing option -c <expression> or -g <filename>" << std::endl;
        return 2;
    }

    std::ifstream f(sourceFile);
    std::stringstream buffer;
    buffer << f.rdbuf();

    std::string source = buffer.str();

    Compiler cmp(CompilerConfig().with<opt::Reporter>(StandartReporter::EmptyReporter));

    ProgramBuilder prog = cmp.parse(sourceFile, source);

    if (!prog) {
        return 3;
    }

    cmp.loadPlugin(STDLIBNAME);

    if (generate) {
        Pipeline ppl = Pipeline::createEmpty().insert(std::shared_ptr<Phase>(new ScopeIdentiferPhase));
        EmptyCollector col;
        cmp.compile(prog, col, ppl);
    } else {
        std::string expr = std::string(exprToComplete);
        std::vector<size_t> scopeId = scopeIdFromStr(std::string(scopeIdStr));
        std::shared_ptr<ScopeFinderPhase> scopeFinderPhase(std::shared_ptr<ScopeFinderPhase>(new ScopeFinderPhase(expr, scopeId)));
        Pipeline ppl = Pipeline::createDefault().insert(scopeFinderPhase).insert(Phase::StopRightAfter("TypeChecking"));
        CtxCollector col;

        cmp.compile(prog, col, ppl);

        if (ast::Expression* exprToComplete = scopeFinderPhase->expressionToComplete()) {
            Completer(col.getCtx()).outputPossibilities(exprToComplete->type(), completionType);
        }
    }

    return 0;
}
