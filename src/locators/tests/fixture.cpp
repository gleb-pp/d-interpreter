#include "fixture.h"

#include <memory>

void RealCodeFixture::SetUp() { file = std::make_shared<locators::CodeFile>("<string>", CODE); }
void RealCodeFixture::TearDown() { file.reset(); }
