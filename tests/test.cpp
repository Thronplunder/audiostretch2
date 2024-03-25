#include <catch2/catch_test_macros.hpp>
#include<vector>
#include "ola.h"
#include "wsola.h"
#include "util.h"
#include "pvtsm.h"



TEST_CASE( "deinterleaving and interleaving", "[deinterleaving, interleavin]" ) {
    std::vector<int> arrayA{1, 1, 2, 2, 3, 3, 4, 4 };
    std::vector<std::vector<int>> arrayB{{0,0,0,0}, {0,0,0,0}};
    std::vector<int> arrayC(8, 0);

    audiostretch::deinterleaveArray<int>(arrayA, arrayB, 2);
    REQUIRE(arrayB[0][0] == arrayB[1][0]);
    REQUIRE(arrayB[0][1] == arrayB[1][1]);
    REQUIRE(arrayB[0][2] == arrayB[1][2]);
    REQUIRE(arrayB[0][3] == arrayB[1][3]);

    audiostretch::interleaveArray(arrayB, arrayC);

    for(int i = 0; i < arrayA.size(); i++){
        REQUIRE(arrayA.at(i) == arrayC.at(i));
    }

}