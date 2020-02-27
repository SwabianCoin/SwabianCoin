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

#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/CryptoHelper/CryptoHelper.h"
#include <gtest/gtest.h>
#include <chrono>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <boost/random.hpp>
using namespace boost::random;

typedef independent_bits_engine<mt19937, 256, scn::hash_t> generator_hash_type;

using namespace scn;


TEST(TestSerialization, SimpleSerialization) {
    int32_t i = 315;

    std::stringstream oss;
    cereal::JSONOutputArchive oa(oss);
    oa << i;
    std::string x = oss.str();

    EXPECT_EQ(oss.str(), "{\n    \"value0\": 315");
}

TEST(TestSerialization, HashSerialization) {
    hash_t hash("0xD428F80D2822CFF295B9746FA9D4D10B497EB3EC0D2B63A562425D69EAB9C5A7");

    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    oa << hash;

    std::stringstream iss;
    iss << oss.str();
    cereal::PortableBinaryInputArchive ia(iss);
    hash_t hash2;
    ia >> hash2;

    EXPECT_EQ(hash, hash2);
}

TEST(TestSerialization, CollectionBlockSerialization) {
    CollectionBlock block;
    block.header.block_uid = 17;
    block.header.generic_header.previous_block_hash = 123;
    block.header.generic_header.block_hash = 789;

    std::stringstream oss;
    cereal::JSONOutputArchive oa(oss);
    oa << block;
    std::string x = oss.str();

    EXPECT_EQ(oss.str(), "{\n    \"value0\": {\n        \"value0\": {\n            \"value0\": {\n                \"value0\": {\n                    \"value0\": {\n                        \"value0\": 0,\n                        \"value1\": 0,\n                        \"value2\": 0,\n                        \"value3\": 789\n                    },\n                    \"value1\": {\n                        \"value0\": 0,\n                        \"value1\": 0,\n                        \"value2\": 0,\n                        \"value3\": 123\n                    },\n                    \"value2\": 1\n                },\n                \"value1\": 17\n            }\n        },\n        \"value1\": [],\n        \"value2\": []\n    }");
}

TEST(TestSerialization, CollectionBlockHashSimple) {
    CollectionBlock block;
    block.header.block_uid = 17;
    block.header.generic_header.previous_block_hash = 123;
    CryptoHelper::fillHash(block);
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "92AB50CAA4A584A6A2953A0DC1E5FBFCD0C2981DE1478241F309BE67A275616D");
}

TEST(TestSerialization, CollectionBlockHashExt) {
    CollectionBlock block;
    block.header.block_uid = 17;
    block.header.generic_header.previous_block_hash = 123;
    TransactionSubBlock trans_block;
    trans_block.header.generic_header.previous_block_hash = 123;
    trans_block.header.generic_header.block_hash = 456;
    trans_block.signature = "SIGN";
    trans_block.fraction = 100;
    trans_block.pre_owner = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\nPre\n-----END PUBLIC KEY-----");
    trans_block.post_owner = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\nPost\n-----END PUBLIC KEY-----");
    block.transactions[trans_block.header.generic_header.block_hash] = trans_block;
    CreationSubBlock creation_block;
    creation_block.header.generic_header.previous_block_hash = 123;
    creation_block.header.generic_header.block_hash = 8484684;
    creation_block.signature = "CSIGN";
    creation_block.data_value = "DATA";
    creation_block.creator = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\nCREATOR\n-----END PUBLIC KEY-----");
    block.creations[creation_block.header.generic_header.block_hash] = creation_block;
    CryptoHelper::fillHash(block);
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "4EC67F2C53743DBF3A8AB740BBDDBC03747D30DDFF390CBC7AA6E891E6D418FC");

    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "4EC67F2C53743DBF3A8AB740BBDDBC03747D30DDFF390CBC7AA6E891E6D418FC");

    block.header.block_uid = 18;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "B504DEEA975CC25FF94693EF83670E7B86E45BB112E3621490AF53413FD312C1");

    block.transactions.begin()->second.fraction++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "39C25ECDD3579899F2B713FB1C95D674DCCE46797E98E7A60D7E2E5A2DA14B4E");
}

std::vector<scn::public_key_t> example_pub_keys = {PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                     "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvY\n"
                                                     "JzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                                     "-----END PUBLIC KEY-----"),
                                                   PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                     "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEMoMRqM5hS3hc3oiocVvjOdlD61FiK/6V\n"
                                                     "CONcprHNBpbQCN52kLAnFVyE7wPu5rCIGspsQtC5zcKqhYidXa48Ew==\n"
                                                     "-----END PUBLIC KEY-----"),
                                                   PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                     "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAExGwxEXQZy6rFPd4NAfs6bJt+uCJxIw4C\n"
                                                     "aamCTSupCTdi/7TwMTD2gUKKD+CBWyYHImXsYtmS3dA+EFsyN24Xuw==\n"
                                                     "-----END PUBLIC KEY-----"),
                                                   PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                     "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEZwZXu5RfYGMxKsC+ZsSyW5pw6zlkoNEj\n"
                                                     "+pGvb1hHNYNRtrBdZUUCpwaoTUlbbC1lae7I44j+UAfSdZR4B6IqgA==\n"
                                                     "-----END PUBLIC KEY-----")};

TEST(TestSerialization, BaselineBlockHashHuge) {
    generator_hash_type random_hash_generator;
    random_hash_generator.seed(1234);
    std::chrono::time_point<std::chrono::system_clock> t1, t2, t3;
    t1 = std::chrono::system_clock::now();
    BaselineBlock block;
    block.header.block_uid = 17;
    block.header.generic_header.previous_block_hash = 123;
    block.mining_state.highest_hash_of_last_epoch = 456;
    block.mining_state.highest_hash_of_current_epoch = 789;
    block.mining_state.num_minings_in_epoch = 42;
    block.mining_state.epoch = 1000;
    uint64_t coins_total = 6000000;
    uint64_t coins_per_pub_key = coins_total / example_pub_keys.size();
    block.data_value_hashes.resize(coins_total/1000);
    for(auto& pub_key : example_pub_keys) {
        block.wallets[pub_key] = coins_per_pub_key;
    }
    for(uint32_t epoch=0;epoch<coins_total/1000;epoch++) {
        block.data_value_hashes[epoch].reserve(1000);
        for(uint32_t creation=0;creation<1000;creation++) {
            block.data_value_hashes[epoch].push_back(random_hash_generator());
        }
    }
    t2 = std::chrono::system_clock::now();
    {
        CryptoHelper::fillHash(block);
    }
    t3 = std::chrono::system_clock::now();
    std::cout << "Generate Data: " << (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count() << "ms" << std::endl
              << "Fill Hash: " << (std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2)).count() << "ms" << std::endl;
    EXPECT_EQ(hash_helper::toString(block.header.generic_header.block_hash), "2736D50FDDC976E766CD1E389E1047E2F7DC6E9448C91CDE045E39371FE272C0");
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);
    return RUN_ALL_TESTS();
}