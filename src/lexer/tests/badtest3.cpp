#include <gtest/gtest.h>

#include "complog/CompilationLog.h"
#include "lexer.h"
#include "locators/locator.h"
using namespace std;

static const char* CODE = "var a := \"hello";

TEST(badtests, unclosed_quote) {
    auto file = make_shared<locators::CodeFile>("BadTest3.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_FALSE(maybeTokens.has_value());
    auto msgs = log.Messages();
    ASSERT_NE(msgs.size(), 0ul);
    bool hasErrors = false;
    bool pointsToEnd = false;
    for (auto& msg : msgs) {
        hasErrors = hasErrors || msg->MessageSeverity() == complog::Severity::Error();
        for (auto& loc : msg->Locators()) pointsToEnd = pointsToEnd || loc.Position() == 15;
    }
    EXPECT_TRUE(hasErrors);
    EXPECT_TRUE(pointsToEnd);
}
