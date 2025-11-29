#include "fixture.h"

#include <memory>

void RealCodeFixture::SetUp() { file = std::make_shared<dinterp::locators::CodeFile>("<string>", CODE); }
void RealCodeFixture::TearDown() { file.reset(); }
