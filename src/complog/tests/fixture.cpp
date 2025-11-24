#include "fixture.h"

#include "dinterp/locators/CodeFile.h"
using namespace std;

void PascalProgram::SetUp() { file = make_shared<locators::CodeFile>("<string>", CODE); }
void PascalProgram::TearDown() { file.reset(); }
