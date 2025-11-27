#include <dinterp/complog/CompilationLog.h>
#include <dinterp/lexer.h>
#include <dinterp/syntax.h>
#include <dinterp/semantic.h>
#include <dinterp/interp.h>
#include <sstream>
using namespace std;

int main() {
    istringstream input;
    ostringstream output;
    dinterp::interp::RuntimeContext context(input, output, 1000, 0);
    dinterp::complog::AccumulatedCompilationLog log;
    auto file = make_shared<dinterp::locators::CodeFile>("<memory>", "print \"Hello, world!\\n\"");
    auto tokens = dinterp::Lexer::tokenize(file, log, true);
    if (!tokens) {
        log.WriteToStream(cerr, dinterp::complog::CompilationMessage::FormatOptions::All(80));
        return 1;
    }
    auto optprogram = dinterp::SyntaxAnalyzer::analyze(*tokens, file, log);
    if (!optprogram) {
        log.WriteToStream(cerr, dinterp::complog::CompilationMessage::FormatOptions::All(80));
        return 1;
    }
    auto program = *optprogram;
    if (!dinterp::semantic::Analyze(log, program)) {
        log.WriteToStream(cerr, dinterp::complog::CompilationMessage::FormatOptions::All(80));
        return 1;
    }
    dinterp::interp::Run(context, *program);
    cout << output.str();
    if (output.str() != "Hello, world!\n") return 1;
    return 0;
}
