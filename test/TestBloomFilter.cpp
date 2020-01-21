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

class TestBloomFilter : public testing::Test {
public:

    TestBloomFilter() {

        random_hash_generator.seed();
        std::srand(123);
    }

    hash_t getRandomHash() {
        return random_hash_generator();
    }

protected:

    generator_hash_type random_hash_generator;
};


TEST_F(TestBloomFilter, BloomFilterInsert) {
    BloomFilter<8192> x;
    x.insertHash(123);
    x.insertHash(456);
    x.insertHash(789);

    EXPECT_EQ(x.findHash(123), true);
    EXPECT_EQ(x.findHash(456), true);
    EXPECT_EQ(x.findHash(789), true);
    EXPECT_EQ(x.findHash(159), false);
}

TEST_F(TestBloomFilter, BloomFilterClear) {
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

TEST_F(TestBloomFilter, BloomFilterEstimate) {
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

TEST_F(TestBloomFilter, BloomFilterMerge) {
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
