#include <gtest/gtest.h>

#include "dinterp/complog/CompilationLog.h"
#include "dinterp/lexer.h"
#include "dinterp/locators/locator.h"
using namespace std;
using namespace dinterp;

static const char* CODE = R"%%(

something something "A string literal
...on multiple lines"
)%%";

TEST(badtests, stringliteral_newline) {
    auto file = make_shared<locators::CodeFile>("BadTest1.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_FALSE(maybeTokens.has_value());
    auto msgs = log.Messages();
    ASSERT_NE(msgs.size(), 0ul);
    bool hasErrors = false;
    bool pointsToNewline = false;
    size_t newlinePosition = file->Position(2, 37);
    for (auto& msg : msgs) {
        hasErrors = hasErrors || msg->MessageSeverity() == complog::Severity::Error();
        for (auto& loc : msg->Locators()) pointsToNewline = pointsToNewline || loc.Position() == newlinePosition;
    }
    EXPECT_TRUE(hasErrors);
    EXPECT_TRUE(pointsToNewline);
}
