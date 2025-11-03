#include "fixture.h"
using namespace std;

TEST_F(RealCodeFixture, LineLengths) {
    ASSERT_EQ(file->LineCount(), 6);
    EXPECT_EQ(file->LineLength(0), 19);
    EXPECT_EQ(file->LineLength(1), 20);
    EXPECT_EQ(file->LineLength(2), 0);
    EXPECT_EQ(file->LineLength(3), 12);
    EXPECT_EQ(file->LineLength(4), 30);
    EXPECT_EQ(file->LineLength(5), 1);
}

TEST_F(RealCodeFixture, PosToLineCol) {
    EXPECT_EQ(file->LineCol(0), make_pair(0ul, 0ul));
    EXPECT_EQ(file->LineCol(45), make_pair(3ul, 3ul));
    EXPECT_EQ(file->LineCol(13), make_pair(0ul, 13ul));
    EXPECT_EQ(file->LineCol(19), make_pair(0ul, 19ul));
    EXPECT_EQ(file->LineCol(20), make_pair(1ul, 0ul));
    EXPECT_EQ(file->LineCol(41), make_pair(2ul, 0ul));
    EXPECT_EQ(file->LineCol(42), make_pair(3ul, 0ul));
    EXPECT_EQ(file->LineCol(string(CODE).size()), make_pair(5ul, 1ul));
}

TEST_F(RealCodeFixture, LineColToPos) {
    EXPECT_EQ(file->Position(0, 0), 0);
    EXPECT_EQ(file->Position(3, 3), 45);
    EXPECT_EQ(file->Position(0, 13), 13);
    EXPECT_EQ(file->Position(0, 19), 19);
    EXPECT_EQ(file->Position(1, 0), 20);
    EXPECT_EQ(file->Position(2, 0), 41);
    EXPECT_EQ(file->Position(3, 0), 42);
    EXPECT_EQ(file->Position(5, 1), string(CODE).size());
}

TEST_F(RealCodeFixture, Filename) { EXPECT_EQ(file->FileName(), "<string>"); }

TEST_F(RealCodeFixture, LineStarts) {
    EXPECT_EQ(file->LineStartPosition(0), 0);
    EXPECT_EQ(file->LineStartPosition(1), 20);
    EXPECT_EQ(file->LineStartPosition(2), 41);
    EXPECT_EQ(file->LineStartPosition(3), 42);
    EXPECT_EQ(file->LineStartPosition(4), 55);
    EXPECT_EQ(file->LineStartPosition(5), 86);
}

TEST_F(RealCodeFixture, LineTexts) {
    EXPECT_EQ(file->LineTextWithoutLineFeed(0), "#include <iostream>");
    EXPECT_EQ(file->LineTextWithoutLineFeed(1), "using namespace std;");
    EXPECT_EQ(file->LineTextWithoutLineFeed(2), "");
    EXPECT_EQ(file->LineTextWithoutLineFeed(3), "int main() {");
    EXPECT_EQ(file->LineTextWithoutLineFeed(4), "    cout << \"Hello, world!\n\";");
    EXPECT_EQ(file->LineTextWithoutLineFeed(5), "}");
    EXPECT_EQ(file->LineTextWithoutLineFeed(6), "}");
}

TEST_F(RealCodeFixture, Contexts) {
    auto cont = file->Context(file->Position(0, 3), 10, 10);
    EXPECT_EQ(cont.PointerWithinText, 3);
    EXPECT_EQ(cont.Text, "#include <ios");
    cont = file->Context(file->Position(1, 10), numeric_limits<size_t>::max(), numeric_limits<size_t>::max());
    EXPECT_EQ(cont.PointerWithinText, 10);
    EXPECT_EQ(cont.Text, "using namespace std;");
    cont = file->Context(file->Position(2, 0), 10, 10);
    EXPECT_EQ(cont.PointerWithinText, 0);
    EXPECT_EQ(cont.Text, "");
    cont = file->Context(file->Position(4, 18), 2, 3);
    EXPECT_EQ(cont.PointerWithinText, 2);
    EXPECT_EQ(cont.Text, "lo, w");
    cont = file->Context(file->Position(4, 29), 10, 10);
    EXPECT_EQ(cont.PointerWithinText, 10);
    EXPECT_EQ(cont.Text, " world!\\n\";");
}
