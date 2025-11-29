#include <gtest/gtest.h>

#include "baderror.h"
#include "badwarning.h"
#include "dinterp/complog/CompilationLog.h"
#include "dinterp/complog/CompilationMessage.h"
#include "dinterp/locators/locator.h"
#include "fixture.h"
using namespace std;
using namespace dinterp;

class WarnAndError : public PascalProgram {
protected:
    shared_ptr<complog::CompilationMessage> warn, err;
    complog::CompilationMessage::FormatOptions options;
    string onlyWarning =
        R"%%%([Warning] (BAD) Bad warning
<string>:
1 |program test;
   ^
)%%%";
    string onlyError =
        R"%%%([Error] (DBLBAD) Very bad
<string>:
3 |var i: Integer;
   ^
<string>:
5 |    for i := 100 downto 1 do
                           ^
)%%%";
    string warningAndError = onlyWarning + '\n' + onlyError;
    virtual void SetUp() override {
        PascalProgram::SetUp();
        warn = make_shared<BadWarning>(locators::Locator(file, 0));
        err = make_shared<DoubleBadError>(locators::Locator(file, file->Position(2, 0)),
                                          locators::Locator(file, file->Position(4, 24)));
        options = complog::CompilationMessage::FormatOptions::All(100);
    }
    virtual void TearDown() override {
        warn.reset();
        err.reset();
        PascalProgram::TearDown();
    }
};

TEST_F(WarnAndError, AccumLogs) {
    complog::AccumulatedCompilationLog log;
    log.Log(warn);
    EXPECT_EQ(log.ToString(options), onlyWarning);
    EXPECT_EQ(log.ToString(complog::Severity::Warning(), options), onlyWarning);
    EXPECT_EQ(log.ToString(complog::Severity::Error(), options), "");
    log.Log(err);
    EXPECT_EQ(log.ToString(options), warningAndError);
    EXPECT_EQ(log.ToString(complog::Severity::Error(), options), onlyError);
    EXPECT_EQ(log.ToString(complog::Severity::Warning(), options), warningAndError);
    EXPECT_EQ(log.ToString(complog::Severity::Info(), options), warningAndError);
}

TEST_F(WarnAndError, StreamLogs) {
    stringstream str1;
    auto log1 = make_shared<complog::StreamingCompilationLog>(str1, options);
    stringstream str2;
    auto log2 = make_shared<complog::StreamingCompilationLog>(str2, options, complog::Severity::Error());
    complog::CombinedCompilationLog logs{log1, log2};
    logs.Log(warn);
    EXPECT_EQ(str1.str(), onlyWarning + '\n');
    EXPECT_EQ(str2.str(), "");
    logs.Log(err);
    EXPECT_EQ(str1.str(), warningAndError + '\n');
    EXPECT_EQ(str2.str(), onlyError + '\n');
}
