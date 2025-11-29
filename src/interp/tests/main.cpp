#include <gtest/gtest.h>

#include "fixture.h"

using namespace std;
using namespace dinterp;
using namespace interp;

inline bool isprime(int n) {
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

TEST_F(Sample, Basic01) {
    ReadFile("samples/basic/01.d", true);
    RunAndExpect("", "5");
}

TEST_F(Sample, Basic02) {
    ReadFile("samples/basic/02.d", true);
    RunAndExpect("", "hello");
}

TEST_F(Sample, Basic03) {
    ReadFile("samples/basic/03.d", true);
    RunAndExpect("", "small");
}

TEST_F(Sample, Basic04) {
    ReadFile("samples/basic/04.d", true);
    RunAndExpect("", "equal");
}

TEST_F(Sample, Basic05) {
    ReadFile("samples/basic/05.d", true);
    RunAndExpect("", "123");
}

TEST_F(Sample, Basic06) {
    ReadFile("samples/basic/06.d", true);
    RunAndExpect("", "5");
}

TEST_F(Sample, Basic07) {
    ReadFile("samples/basic/07.d", true);
    RunAndExpect("", "123");
}

TEST_F(Sample, Basic08) {
    ReadFile("samples/basic/08.d", true);
    RunAndExpect("", "102030");
}

TEST_F(Sample, Basic09) {
    ReadFile("samples/basic/09.d", true);
    RunAndExpect("", "100200");
}

TEST_F(Sample, Basic10) {
    ReadFile("samples/basic/10.d", true);
    RunAndExpect("", "1hi");
}

TEST_F(Sample, Basic11) {
    ReadFile("samples/basic/11.d", true);
    RunAndExpect("", "12");
}

TEST_F(Sample, Basic12) {
    ReadFile("samples/basic/12.d", true);
    RunAndExpect("", "11");
}

TEST_F(Sample, Basic13) {
    ReadFile("samples/basic/13.d", true);
    RunAndExpect("", "ok");
}

TEST_F(Sample, Basic14) {
    ReadFile("samples/basic/14.d", true);
    RunAndExpect("", "true");
}

TEST_F(Sample, Basic15) {
    ReadFile("samples/basic/15.d", true);
    RunAndExpect("", "[ [1] <closure: function (object?) -> object?> ]");
}

TEST_F(Sample, Complex01) {
    ReadFile("samples/complex/01.d", true);
    stringstream primes;
    for (int i = 2; i < 100; i++)
        if (isprime(i)) primes << i << '\n';
    string sprimes = primes.str();
    RunAndExpect("", sprimes.c_str());
}

TEST_F(Sample, Complex02) {
    ReadFile("samples/complex/02.d", true);
    RunAndExpect("", R"(1
1
2
3
5
8
13
21
34
55
89
144
233
377
610
987
1597
2584
4181
6765
10946
17711
28657
46368
75025
121393
196418
317811
514229
832040
1346269
2178309
3524578
5702887
9227465
14930352
24157817
39088169
63245986
102334155
165580141
267914296
433494437
701408733
1134903170
1836311903
2971215073
4807526976
7778742049
12586269025
20365011074
32951280099
53316291173
86267571272
139583862445
225851433717
365435296162
591286729879
956722026041
1548008755920
2504730781961
4052739537881
6557470319842
10610209857723
17167680177565
27777890035288
44945570212853
72723460248141
117669030460994
190392490709135
308061521170129
498454011879264
806515533049393
1304969544928657
2111485077978050
3416454622906707
5527939700884757
8944394323791464
14472334024676221
23416728348467685
37889062373143906
61305790721611591
99194853094755497
160500643816367088
259695496911122585
420196140727489673
679891637638612258
1100087778366101931
1779979416004714189
2880067194370816120
4660046610375530309
7540113804746346429
12200160415121876738
19740274219868223167
31940434634990099905
51680708854858323072
83621143489848422977
135301852344706746049
218922995834555169026
354224848179261915075
)");
}

TEST_F(Sample, Complex03) {
    ReadFile("samples/complex/03.d", true);
    RunAndExpect("",
                 "100! = "
                 "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286"
                 "253697920827223758251185210916864000000000000000000000000");
}

TEST_F(Sample, Complex04) {
    ReadFile("samples/complex/04.d", true);
    RunAndExpect("", R"(normal
===-----===
decorated
===-----===
===-----===
Call: 8 800 123 45 67
===-----===
)");
}

TEST_F(Sample, Complex05) { ReadFile("samples/complex/05.d", false); }

TEST_F(Sample, Complex06) {
    ReadFile("samples/complex/06.d", true);
    RunAndExpect("", "[ [1] initial, [2] list, [3] item, [4] item, [5] item, [6] item, [7] item, [8] item ]");
}

TEST_F(Sample, Complex07) {
    ReadFile("samples/complex/07.d", true);
    RunAndExpect("", "truetruetruetruetruetruetrue");
}

TEST_F(Sample, Complex08) {
    ReadFile("samples/complex/08.d", true);
    RunAndExpect("", R"({
    e := 80
    2 := 98
    3 := -1
}80)");
}

TEST_F(Sample, Complex09) {
    ReadFile("samples/complex/09.d", true);
    RunAndExpect("", "");
}

TEST_F(Sample, Complex10) {
    ReadFile("samples/complex/10.d", true);
    RunAndExpect("", "");
}

TEST_F(Sample, Complex11) { ReadFile("samples/complex/11.d", false); }

TEST_F(Sample, Complex12) {
    ReadFile("samples/complex/12.d", true);
    RunAndExpect("", "0123456789true");
}
TEST_F(Sample, Complex13) {
    ReadFile("samples/complex/13.d", true);
    RunAndExpect("", R"( 802.11
                      ##################                      
                ######                  ######                
            ####                              ####            
          ##                                      ##          
      ####                                          ####      
    ##                  ##############                  ##    
                  ######              ######                  
                ##                          ##                
            ####                              ####            
                                                              
                          ##########                          
                      ####          ####                      
                  ####                  ####                  
                                                              
                                                              
                            ######                            
                          ##      ##                          
                                                              
                                                              
                              ##                              
)");
}

TEST_F(Sample, Complex14) {
    ReadFile("samples/complex/14.d", true);
    RunAndExpect("", "truetrue");
}

TEST_F(Sample, Complex15) {
    ReadFile("samples/complex/15.d", true);
    auto& sts = program->statements;
    vector<shared_ptr<ast::Statement>> badstatements(sts.begin() + 3, sts.end());
    sts.erase(sts.begin() + 2, sts.end());
    for (auto& stmt : badstatements) {
        sts.push_back(stmt);
        RunAndExpectCrash("");
        sts.pop_back();
    }
}

TEST_F(Sample, ExtraArray) {
    ReadFile("samples/extra/array.d", true);
    RunAndExpect("", R"([ [1] a, [3] c, [10] d ]
false
[ [1] 1, [2] 3, [3] 10 ]
)");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
