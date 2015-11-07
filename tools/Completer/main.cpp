#include <fstream>
#include "Compiler/Frontend/AST/Nodes/Program.h"
#include "Compiler/Frontend/AST/Visitors/ASTPrinter.h"
#include "Compiler/Frontend/Parser/Parser.h"
#include "Compiler/Frontend/Symbols/Scope.h"
#include "Visitors/ScopeIdentifier.h"
#include "ScopeReporter.h"
#include "sfsl.h"
#include "unistd.h"

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

        complete::ScopeFinder is(ctx, _scopeId);
        prog->onVisit(&is);

        ast::Block* blk = is.getBlock();

        if (!blk) {
            return false;
        }

        src::StringSource source(src::InputSourceName::make(ctx, "tmp"), _toComplete);
        lex::Lexer toCompleteLexer(ctx, source);
        ast::Parser toCompleteParser(ctx, toCompleteLexer);

        _exprToComplete = toCompleteParser.parseSingleExpression();

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
                ast::ClassDecl* clss = pt->getClass();
                sym::Scope* clssScope = clss->getScope();

                for (const std::pair<std::string, sym::SymbolData>& entry : clssScope->getAllSymbols()) {
                    outputFromSymbolData(entry.second, tp->applyTCCallsOnly(_ctx)->getSubstitutionTable());
                }
            }
        }
    }

private:

    void outputFromSymbolData(const sym::SymbolData& data, const type::SubstitutionTable& table) {
        type::Type* symType = nullptr;

        switch (data.symbol->getSymbolType()) {
        case sym::SYM_VAR:
            symType = static_cast<sym::VariableSymbol*>(data.symbol)->type();
            break;
        case sym::SYM_DEF:
            symType = static_cast<sym::DefinitionSymbol*>(data.symbol)->type();
            break;
        default:
            break;
        }

        if (!symType) {
            return;
        }

        symType = type::Type::findSubstitution(data.env, symType)->substitute(data.env, _ctx);
        symType = type::Type::findSubstitution(table, symType)->substitute(table, _ctx);

        std::cout << data.symbol->getName() << ":" << symType->apply(_ctx)->toString() << std::endl;
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

    std::string source;

    std::ifstream f(sourceFile);
    while (f.good()) {
        source += f.get();
    }

    Compiler cmp(CompilerConfig(StandartReporter::EmptyReporter));

    ProgramBuilder prog = cmp.parse(sourceFile, source);

    if (!prog) {
        return 3;
    }

    Module slang = prog.openModule("sfsl").openModule("lang");
    slang.typeDef("unit", cmp.classBuilder("unit").build());
    slang.typeDef("bool", cmp.classBuilder("bool").build());
    slang.typeDef("int", cmp.classBuilder("int").build());
    slang.typeDef("real", cmp.classBuilder("real").build());
    slang.typeDef("string", cmp.classBuilder("string").build());

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

        Completer(col.getCtx()).outputPossibilities(scopeFinderPhase->expressionToComplete()->type(), completionType);
    }

    return 0;
}
