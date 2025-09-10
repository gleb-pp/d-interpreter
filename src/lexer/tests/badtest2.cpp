#include <gtest/gtest.h>
#include "locators/locator.h"
#include "complog/CompilationLog.h"
#include "lexer.h"
using namespace std;

static const char* CODE = R"%%(
:= := := : := := :=
)%%";

TEST(badtests, invalid_colon) {
    auto file = make_shared<locators::CodeFile>("BadTest2.d", CODE);
    complog::AccumulatedCompilationLog log;
    auto maybeTokens = Lexer::tokenize(file, log);
    ASSERT_FALSE(maybeTokens.has_value());
    auto msgs = log.Messages();
    ASSERT_NE(msgs.size(), 0ul);
    bool hasErrors = false;
    bool pointsToColon = false;
    size_t colonPosition = file->Position(1, 9);
    for (auto& msg : msgs) {
        hasErrors = hasErrors || msg->MessageSeverity() == complog::Severity::Error();
        for (auto& loc : msg->Locators())
            pointsToColon = pointsToColon || loc.Position() == colonPosition;
    }
    EXPECT_TRUE(hasErrors);
    EXPECT_TRUE(pointsToColon);
}
