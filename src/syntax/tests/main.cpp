#include "fixture.h"
#include <gtest/gtest.h>
using namespace std;

TEST_F(FileSample, Basic01) { ReadFile("basic/01.d", true); }
TEST_F(FileSample, Basic02) { ReadFile("basic/02.d", true); }
TEST_F(FileSample, Basic03) { ReadFile("basic/03.d", true); }
TEST_F(FileSample, Basic04) { ReadFile("basic/04.d", true); }
TEST_F(FileSample, Basic05) { ReadFile("basic/05.d", true); }
TEST_F(FileSample, Basic06) { ReadFile("basic/06.d", true); }
TEST_F(FileSample, Basic07) { ReadFile("basic/07.d", true); }
TEST_F(FileSample, Basic08) { ReadFile("basic/08.d", true); }
TEST_F(FileSample, Basic09) { ReadFile("basic/09.d", true); }
TEST_F(FileSample, Basic10) { ReadFile("basic/10.d", true); }
TEST_F(FileSample, Basic11) { ReadFile("basic/11.d", true); }
TEST_F(FileSample, Basic12) { ReadFile("basic/12.d", true); }
TEST_F(FileSample, Basic13) { ReadFile("basic/13.d", true); }
TEST_F(FileSample, Basic14) { ReadFile("basic/14.d", true); }
TEST_F(FileSample, Basic15) { ReadFile("basic/15.d", true); }

TEST_F(FileSample, Complex01) { ReadFile("complex/01.d", true); }
TEST_F(FileSample, Complex02) { ReadFile("complex/02.d", true); }
TEST_F(FileSample, Complex03) { ReadFile("complex/03.d", true); }
TEST_F(FileSample, Complex04) { ReadFile("complex/04.d", true); }
TEST_F(FileSample, Complex05) {
    ReadFile("complex/05.d", false);
    ExpectFailure(3, 16);
}
TEST_F(FileSample, Complex06) { ReadFile("complex/06.d", true); }
TEST_F(FileSample, Complex07) { ReadFile("complex/07.d", true); }
TEST_F(FileSample, Complex08) { ReadFile("complex/08.d", true); }
TEST_F(FileSample, Complex09) { ReadFile("complex/09.d", true); }
TEST_F(FileSample, Complex10) { ReadFile("complex/10.d", true); }
TEST_F(FileSample, Complex11) { ReadFile("complex/11.d", true); }
TEST_F(FileSample, Complex12) { ReadFile("complex/12.d", true); }
TEST_F(FileSample, Complex13) { ReadFile("complex/13.d", true); }
TEST_F(FileSample, Complex14) { ReadFile("complex/14.d", true); }
TEST_F(FileSample, Complex15) { ReadFile("complex/15.d", true); }

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
