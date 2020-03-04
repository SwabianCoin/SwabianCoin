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

#include "scn/Common/BloomFilter.h"
#include <gtest/gtest.h>
#include <boost/random.hpp>
using namespace boost::random;

using namespace scn;

typedef independent_bits_engine<mt19937, 256, scn::hash_t> generator_hash_type;

class TestCommon : public testing::Test {
public:

    TestCommon() {

        random_hash_generator.seed();
        std::srand(123);
    }

    hash_t getRandomHash() {
        return random_hash_generator();
    }

protected:

    std::string example_owner_public_key_string = "-----BEGIN PUBLIC KEY-----\n"
                                                  "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvY\n"
                                                  "JzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                                  "-----END PUBLIC KEY-----";

    std::string example_owner_public_key_string_short = "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==";

    generator_hash_type random_hash_generator;
};


TEST_F(TestCommon, HashHelperToString) {
    EXPECT_EQ(hash_helper::toString(0xABCDEF), "ABCDEF");
}

TEST_F(TestCommon, HashHelperToArray) {
    uint8_t array[32];
    hash_helper::toArray(0x123456789ABCDEF, array);
    EXPECT_EQ(array[0], 0x00);
    EXPECT_EQ(array[1], 0x00);
    EXPECT_EQ(array[2], 0x00);
    EXPECT_EQ(array[3], 0x00);
    EXPECT_EQ(array[4], 0x00);
    EXPECT_EQ(array[5], 0x00);
    EXPECT_EQ(array[6], 0x00);
    EXPECT_EQ(array[7], 0x00);
    EXPECT_EQ(array[8], 0x00);
    EXPECT_EQ(array[9], 0x00);
    EXPECT_EQ(array[11], 0x00);
    EXPECT_EQ(array[10], 0x00);
    EXPECT_EQ(array[12], 0x00);
    EXPECT_EQ(array[13], 0x00);
    EXPECT_EQ(array[14], 0x00);
    EXPECT_EQ(array[15], 0x00);
    EXPECT_EQ(array[16], 0x00);
    EXPECT_EQ(array[17], 0x00);
    EXPECT_EQ(array[18], 0x00);
    EXPECT_EQ(array[19], 0x00);
    EXPECT_EQ(array[20], 0x00);
    EXPECT_EQ(array[21], 0x00);
    EXPECT_EQ(array[22], 0x00);
    EXPECT_EQ(array[23], 0x00);
    EXPECT_EQ(array[24], 0x01);
    EXPECT_EQ(array[25], 0x23);
    EXPECT_EQ(array[26], 0x45);
    EXPECT_EQ(array[27], 0x67);
    EXPECT_EQ(array[28], 0x89);
    EXPECT_EQ(array[29], 0xAB);
    EXPECT_EQ(array[30], 0xCD);
    EXPECT_EQ(array[31], 0xEF);
}

TEST_F(TestCommon, HashHelperFromArray) {
    uint8_t array[32] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    hash_t hash = hash_helper::fromArray(array);
    EXPECT_EQ(hash, 0x123456789ABCDEF);
}


TEST_F(TestCommon, PublicKeyConstruct1) {
    PublicKeyPEM key(example_owner_public_key_string);
    EXPECT_EQ(key.getAsShortString(), example_owner_public_key_string_short);
    EXPECT_EQ(key.isEmpty(), false);
}

TEST_F(TestCommon, PublicKeyConstruct2) {
    {
        std::ofstream ofs("key");
        ofs << example_owner_public_key_string;
    }
    std::ifstream ifs("key");
    PublicKeyPEM key(ifs);
    EXPECT_EQ(key.getAsShortString(), example_owner_public_key_string_short);
    EXPECT_EQ(key.isEmpty(), false);
}

TEST_F(TestCommon, PublicKeyConstruct3) {
    PublicKeyPEM key("");
    EXPECT_EQ(key.isEmpty(), true);
}


TEST_F(TestCommon, BloomFilterInsert) {
    BloomFilter<8192> x;
    x.insertHash(123);
    x.insertHash(456);
    x.insertHash(789);

    EXPECT_EQ(x.findHash(123), true);
    EXPECT_EQ(x.findHash(456), true);
    EXPECT_EQ(x.findHash(789), true);
    EXPECT_EQ(x.findHash(159), false);
}

TEST_F(TestCommon, BloomFilterClear) {
    BloomFilter<8192> x;
    x.insertHash(123);
    x.insertHash(456);
    x.insertHash(789);
    x.clear();

    EXPECT_EQ(x.findHash(123), false);
    EXPECT_EQ(x.findHash(456), false);
    EXPECT_EQ(x.findHash(789), false);
    EXPECT_EQ(x.findHash(159), false);
}

TEST_F(TestCommon, BloomFilterEstimate) {
    BloomFilter<8192> x;
    auto est = x.numHashEstimation();
    EXPECT_NEAR(x.numHashEstimation(), 0, 0.5);
    x.insertHash(getRandomHash());
    auto est1 = x.numHashEstimation();
    EXPECT_NEAR(x.numHashEstimation(), 1, 0.5);
    x.insertHash(getRandomHash());
    auto est2 = x.numHashEstimation();
    EXPECT_NEAR(x.numHashEstimation(), 2, 0.5);
    x.insertHash(getRandomHash());
    auto est3 = x.numHashEstimation();
    EXPECT_NEAR(x.numHashEstimation(), 3, 0.5);
}

TEST_F(TestCommon, BloomFilterMerge) {
    BloomFilter<8192> a, b;

    a.insertHash(123);
    a.insertHash(456);
    b.insertHash(789);

    a.merge(b);

    EXPECT_EQ(a.findHash(123), true);
    EXPECT_EQ(a.findHash(456), true);
    EXPECT_EQ(a.findHash(789), true);
    EXPECT_EQ(a.findHash(159), false);

    EXPECT_EQ(b.findHash(123), false);
    EXPECT_EQ(b.findHash(456), false);
    EXPECT_EQ(b.findHash(789), true);
    EXPECT_EQ(b.findHash(159), false);
}
