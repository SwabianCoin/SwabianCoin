/*
 * This file is part of SwabianCoin.
 *
 * SwabianCoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * SwabianCoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SwabianCoin.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "scn/CryptoHelper/CryptoHelper.h"
#include <gtest/gtest.h>

using namespace scn;

TEST(TestCrypto, HashCalculationShort) {
    auto hash = CryptoHelper::calcHash("A random text.");
    EXPECT_EQ(hash_helper::toString(hash), "1FF38799705BD4B5CEAA66EE3DEC54A304E16B64A1711838730EFF6752F2BA6");
}

TEST(TestCrypto, HashCalculationShortStream) {
    std::stringstream ss;
    ss << "A random text.";
    auto hash = CryptoHelper::calcHash(ss);
    EXPECT_EQ(hash_helper::toString(hash), "1FF38799705BD4B5CEAA66EE3DEC54A304E16B64A1711838730EFF6752F2BA6");
}

void buildLongStringStream(std::stringstream& ss) {
    //about 100 MB
    for(auto i=0;i<7500000;i++) {
        ss << "A random text.";
    }
}

TEST(TestCrypto, HashCalculationLong) {
    std::stringstream ss;
    buildLongStringStream(ss);
    auto hash = CryptoHelper::calcHash(ss.str());
    EXPECT_EQ(hash_helper::toString(hash), "D428F80D2822CFF295B9746FA9D4D10B497EB3EC0D2B63A562425D69EAB9C5A7");
}

TEST(TestCrypto, HashCalculationLongStream) {
    std::stringstream ss;
    buildLongStringStream(ss);
    auto hash = CryptoHelper::calcHash(ss);
    EXPECT_EQ(hash_helper::toString(hash), "D428F80D2822CFF295B9746FA9D4D10B497EB3EC0D2B63A562425D69EAB9C5A7");
}
