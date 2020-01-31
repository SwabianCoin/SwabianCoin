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
#include "scn/Blockchain/Blockchain.h"
#include <gtest/gtest.h>

using namespace scn;


class TestBlockchain : public testing::Test {
public:

    TestBlockchain()
    : crypto(example_owner_public_key, example_owner_private_key)
    , blockchain("./blockchain/") {
        blockchain.initEmptyChain();
    }

    CreationSubBlock buildCreationSubBlock(const std::string& data_value) {
        auto previous_block = blockchain.getNewestBlock();
        CreationSubBlock creation_sub_block;
        creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        creation_sub_block.creator = example_owner_public_key;
        creation_sub_block.data_value = data_value;
        crypto.fillSignature(creation_sub_block);
        CryptoHelper::fillHash(creation_sub_block);
        return creation_sub_block;
    }

    TransactionSubBlock buildTransactionSubBlock(const std::pair<public_key_t, uint64_t>& transaction) {
        auto previous_block = blockchain.getNewestBlock();
        TransactionSubBlock transaction_sub_block;
        transaction_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        transaction_sub_block.pre_owner = example_owner_public_key;
        transaction_sub_block.post_owner = transaction.first;
        transaction_sub_block.fraction = transaction.second;
        crypto.fillSignature(transaction_sub_block);
        CryptoHelper::fillHash(transaction_sub_block);
        return transaction_sub_block;
    }

    CollectionBlock buildCollectionBlock(const std::vector<std::string>& data_values, const std::vector<std::pair<public_key_t, uint64_t>>& transactions) {
        auto previous_block = blockchain.getNewestBlock();

        CollectionBlock collection_block;
        collection_block.header.block_uid = previous_block->header.block_uid + 1;
        collection_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;

        for(auto& data_value : data_values) {
            auto creation_sub_block = buildCreationSubBlock(data_value);
            collection_block.creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
        }

        for(auto& transaction : transactions) {
            auto transaction_sub_block = buildTransactionSubBlock(transaction);
            collection_block.transactions[transaction_sub_block.header.generic_header.block_hash] = transaction_sub_block;
        }

        CryptoHelper::fillHash(collection_block);
        return collection_block;
    }

    void addBlock(const std::vector<std::string>& data_values, const std::vector<std::pair<public_key_t, uint64_t>>& transactions) {
        blockchain.addBlock(buildCollectionBlock(data_values, transactions));
    }

    BaselineBlock buildBaselineBlock(bool add_some_coins = true, block_uid_t block_uid = 721, hash_t previous_block_hash = 0) {
        BaselineBlock baseline_block;
        baseline_block.header.block_uid = block_uid;
        baseline_block.header.generic_header.previous_block_hash = previous_block_hash;

        baseline_block.mining_state.epoch = 0;
        baseline_block.mining_state.highest_hash_of_current_epoch = 0;
        baseline_block.mining_state.highest_hash_of_last_epoch = 0;
        baseline_block.mining_state.num_minings_in_epoch = 0;

        baseline_block.data_value_hashes.resize(1);
        if(add_some_coins) {
            baseline_block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
            baseline_block.data_value_hashes[0].push_back(CryptoHelper::calcHash(valid_data_values_epoch_0[0]));
            baseline_block.mining_state.num_minings_in_epoch++;
            baseline_block.mining_state.highest_hash_of_current_epoch = std::max(baseline_block.mining_state.highest_hash_of_current_epoch,  baseline_block.data_value_hashes[0].back());

            baseline_block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
            baseline_block.data_value_hashes[0].push_back(CryptoHelper::calcHash(valid_data_values_epoch_0[1]));
            baseline_block.mining_state.num_minings_in_epoch++;
            baseline_block.mining_state.highest_hash_of_current_epoch = std::max(baseline_block.mining_state.highest_hash_of_current_epoch,  baseline_block.data_value_hashes[0].back());

            baseline_block.wallets[other_public_key] += TransactionSubBlock::fraction_per_coin;
            baseline_block.data_value_hashes[0].push_back(CryptoHelper::calcHash(valid_data_values_epoch_0[2]));
            baseline_block.mining_state.num_minings_in_epoch++;
            baseline_block.mining_state.highest_hash_of_current_epoch = std::max(baseline_block.mining_state.highest_hash_of_current_epoch,  baseline_block.data_value_hashes[0].back());

            baseline_block.wallets[other_public_key] += TransactionSubBlock::fraction_per_coin;
            baseline_block.data_value_hashes[0].push_back(CryptoHelper::calcHash(valid_data_values_epoch_0[3]));
            baseline_block.mining_state.num_minings_in_epoch++;
            baseline_block.mining_state.highest_hash_of_current_epoch = std::max(baseline_block.mining_state.highest_hash_of_current_epoch,  baseline_block.data_value_hashes[0].back());
        }
        std::sort(baseline_block.data_value_hashes[0].begin(), baseline_block.data_value_hashes[0].end());

        CryptoHelper::fillHash(baseline_block);
        return baseline_block;
    }

protected:

    public_key_t example_owner_public_key = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                            "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvY\n"
                                            "JzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                            "-----END PUBLIC KEY-----");

    private_key_t example_owner_private_key = "-----BEGIN EC PRIVATE KEY-----\n"
                                              "MHQCAQEEIGm1P/iWsWSXlGCLSmokqRN3yKjx5HujGNjCkKOMF21poAcGBSuBBAAK\n"
                                              "oUQDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqT\n"
                                              "XyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                              "-----END EC PRIVATE KEY-----";

    public_key_t other_public_key = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                    "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEMoMRqM5hS3hc3oiocVvjOdlD61FiK/6V\n"
                                    "CONcprHNBpbQCN52kLAnFVyE7wPu5rCIGspsQtC5zcKqhYidXa48Ew==\n"
                                    "-----END PUBLIC KEY-----");

    public_key_t other_public_key_2 = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                      "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAExGwxEXQZy6rFPd4NAfs6bJt+uCJxIw4C\n"
                                      "aamCTSupCTdi/7TwMTD2gUKKD+CBWyYHImXsYtmS3dA+EFsyN24Xuw==\n"
                                      "-----END PUBLIC KEY-----");

    public_key_t example_owner_public_key_modified = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                         "   MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==   \n"
                                                         "-----END PUBLIC KEY-----\n\n\n");

    CryptoHelper crypto;
    Blockchain blockchain;

    std::vector<std::string> valid_data_values_epoch_0 = {
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_7B",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_618",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_6D7",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_947",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_95A",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_A52"
    };
};


TEST_F(TestBlockchain, getBalance) {
    addBlock({valid_data_values_epoch_0[1], valid_data_values_epoch_0[2]}, {}); //add some balance
    EXPECT_EQ(blockchain.getBalance(example_owner_public_key), 2000000);
}

TEST_F(TestBlockchain, getNumWallets) {
    EXPECT_EQ(blockchain.getNumWallets(), 0);
    addBlock({valid_data_values_epoch_0[1], valid_data_values_epoch_0[2]}, {}); //add some balance
    EXPECT_EQ(blockchain.getNumWallets(), 1);
    blockchain.setRootBlock(buildBaselineBlock());
    EXPECT_EQ(blockchain.getNumWallets(), 2);
}

TEST_F(TestBlockchain, nonIntrusiveGetBalance) {

    blockchain.initEmptyChain();
    blockchain.establishBaseline();
    hash_t hash1 = blockchain.getNewestBlock()->header.generic_header.block_hash;

    blockchain.initEmptyChain();
    blockchain.getBalance(PublicKeyPEM("-----BEGIN PUBLIC KEY-----\nSome fake public key to check balance - This should not change the baseline hash, even if the public key does not exist.\n-----END PUBLIC KEY-----"));
    blockchain.establishBaseline();
    hash_t hash2 = blockchain.getNewestBlock()->header.generic_header.block_hash;

    EXPECT_EQ(hash1, hash2);
}

TEST_F(TestBlockchain, getBalanceModifiedPublicKey) {
    addBlock({valid_data_values_epoch_0[1], valid_data_values_epoch_0[2]}, {}); //add some balance
    EXPECT_EQ(blockchain.getBalance(example_owner_public_key_modified), 2000000);
}

TEST_F(TestBlockchain, HashAreaCalculation) {
    hash_t max_allowed_hash, min_allowed_hash;

    Blockchain::getHashArea(1, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "FF8BCFF3EDF1A8D2A0B4C276EA5769595237ADB6C2B194E46E60508026B3F88A");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "FF17D4A36FD835FB55D7162AC718802635934A167AEBC33A24F3B15EC9E1D7EC");

    Blockchain::getHashArea(2, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "FF17D4A36FD835FB55D7162AC718802635934A167AEBC33A24F3B15EC9E1D7EB");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "FEA40DF696CEF441A97C7A4364F3D15959B48EA4C4598F516C2A861391326FE8");

    Blockchain::getHashArea(10, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "FB7F59513683B463ED042DE87F9A854A074FDF5D7DD21A51D9084BCAF6F4D29B");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "FB0D346901428BCB7D1BDF4B3F20F6FF9B683A8A749BED1A609BB630AA3E046A");

    Blockchain::getHashArea(100, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "D66045854E95A666A09AC8A4DFC65C23CD709A96E6547A50601945233625E6F0");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "D5FEF9A9A583027398B8E406D8FD3FAEE07A27922DBE6FF2917FFB2335DEC256");

    Blockchain::getHashArea(1000, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "2B694D7A8004BB80D67B6B87343ED2DA2A5CCAA60817ADB2B847D4050998E48A");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "2B55999D99FC499FA805DDD45D78775E1F217405AFB3110EAF3987EF0E4CFA6A");

    Blockchain::getHashArea(10000, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "5472D14EE5C57F9ADF9C16F6669995B78C186971500CF8AE48EE0FF907");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "544C7D6E8B6FF994450742B5B1351991323A12D41B062F69C0A33022B6");

    Blockchain::getHashArea(50000, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "1000000000717EE28BDCEB8D62AB0E5A5");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "FF8BCFF3F5065ECEA1C697A964488DBB");

    Blockchain::getHashArea(92536, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "89DD9");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "899F0");

    Blockchain::getHashArea(92537, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "899EF");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "89608");

    Blockchain::getHashArea(92538, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "89607");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "89222");

    Blockchain::getHashArea(95000, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "1AC3");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "1AB7");

    Blockchain::getHashArea(96755, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "1");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "1");

    Blockchain::getHashArea(96756, max_allowed_hash, min_allowed_hash);
    EXPECT_EQ(hash_helper::toString(max_allowed_hash), "0");
    EXPECT_EQ(hash_helper::toString(min_allowed_hash), "0");
}


TEST_F(TestBlockchain, validateCollectionBlockValid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    EXPECT_TRUE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockInvalidHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    block.header.generic_header.block_hash++;
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockInvalidPrevBlockHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    block.header.generic_header.previous_block_hash++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockInvalidBlockType) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    block.header.generic_header.block_type = BlockType::BaselineBlock;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockInvalidBlockUid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    block.header.block_uid = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockWrongBlockUid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {{other_public_key, 1700}});
    block.header.block_uid++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}



TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidHash) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.header.generic_header.block_hash++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidPrevBlockHash) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.header.generic_header.previous_block_hash++;
    block.creations.begin()->second.signature.clear();
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.creations.begin()->second);
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidBlockType) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.header.generic_header.block_type = BlockType::TransactionSubBlock;
    block.creations.begin()->second.signature.clear();
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.creations.begin()->second);
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidSignature) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.signature.append("x");
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidDataValue) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.data_value.append("F");
    block.creations.begin()->second.signature.clear();
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.creations.begin()->second);
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationWrongCreator) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    block.creations.begin()->second.creator = other_public_key;
    block.creations.begin()->second.signature.clear();
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.creations.begin()->second);
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationInvalidCreator) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    auto modified_string = block.creations.begin()->second.creator.getAsFullString();
    modified_string[50] = 'D';
    block.creations.begin()->second.creator = PublicKeyPEM(modified_string);
    block.creations.begin()->second.signature.clear();
    block.creations.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.creations.begin()->second);
    CryptoHelper::fillHash(block.creations.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationDuplicateDataValue) {
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0], valid_data_values_epoch_0[1], valid_data_values_epoch_0[0]}, {});
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockCreationDuplicateDataValue2) {
    addBlock(valid_data_values_epoch_0, {}); //add some data values
    auto block = buildCollectionBlock({valid_data_values_epoch_0[0]}, {});
    EXPECT_FALSE(blockchain.validateBlock(block));
}


TEST_F(TestBlockchain, validateCollectionBlockTransactionNotEnoughBalance) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 5000000}});
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionNotEnoughBalance2) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 800000}, {other_public_key_2, 800000}});
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionEnoughBalance) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({valid_data_values_epoch_0[2]}, {{other_public_key, 800000}, {other_public_key_2, 800000}});
    EXPECT_TRUE(blockchain.validateBlock(block));
    blockchain.addBlock(block);
    EXPECT_EQ(blockchain.getBalance(example_owner_public_key), 400000);
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidFraction) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 0}});
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.header.generic_header.block_hash++;
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidPrevBlockHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.header.generic_header.previous_block_hash++;
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidBlockType) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.header.generic_header.block_type = BlockType::CreationSubBlock;
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidSignature) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.signature.append("x");
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionEqualPrePostOwner) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{example_owner_public_key, 1700}});
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionWrongPreOwner) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.pre_owner = other_public_key_2;
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidPreOwner) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    auto modified_string = block.transactions.begin()->second.pre_owner.getAsFullString();
    modified_string[50] = 'D';
    block.transactions.begin()->second.pre_owner = PublicKeyPEM(modified_string);
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionEmptyPostOwner) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    block.transactions.begin()->second.post_owner = PublicKeyPEM("");
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidPostOwner1) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    auto modified_string = block.transactions.begin()->second.post_owner.getAsFullString();
    modified_string[5]++;
    block.transactions.begin()->second.post_owner = PublicKeyPEM(modified_string);
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidPostOwner2) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    auto modified_string = block.transactions.begin()->second.post_owner.getAsFullString();
    modified_string[50]++;
    block.transactions.begin()->second.post_owner = PublicKeyPEM(modified_string);
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateCollectionBlockTransactionInvalidPostOwner3) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add some balance
    auto block = buildCollectionBlock({}, {{other_public_key, 1700}});
    auto modified_string = block.transactions.begin()->second.post_owner.getAsFullString();
    modified_string.resize(modified_string.length() - 2);
    block.transactions.begin()->second.post_owner = PublicKeyPEM(modified_string);
    block.transactions.begin()->second.signature.clear();
    block.transactions.begin()->second.header.generic_header.block_hash = 0;
    block.header.generic_header.block_hash = 0;
    crypto.fillSignature(block.transactions.begin()->second);
    CryptoHelper::fillHash(block.transactions.begin()->second);
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}


TEST_F(TestBlockchain, validateBaselineBlockValidNoContext) {
    auto block = buildBaselineBlock();
    EXPECT_TRUE(blockchain.validateBlockWithoutContext(block));
}

TEST_F(TestBlockchain, validateBaselineBlockValid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    EXPECT_TRUE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockInvalidHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.header.generic_header.block_hash++;
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockInvalidPrevBlockHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.header.generic_header.previous_block_hash++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockInvalidBlockType) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.header.generic_header.block_type = BlockType::CollectionBlock;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockInvalidBlockUid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.header.block_uid = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWrongBlockUid) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.header.block_uid++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockInvalidDataValueHash) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.data_value_hashes[0].push_back(1234);
    std::sort(block.data_value_hashes[0].begin(), block.data_value_hashes[0].end());
    block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockUnsortedDataValueHashes) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    auto temp = block.data_value_hashes[0][1];
    block.data_value_hashes[0][1] = block.data_value_hashes[0][2];
    block.data_value_hashes[0][2] = temp;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockMismatchBetweenWalletsAndHashes) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.data_value_hashes[0].erase(block.data_value_hashes[0].begin()+2);
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockExceedMaxCurrentEpochSize) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    for(uint32_t i=0;i<1050;i++) {
        block.data_value_hashes[0].push_back(block.data_value_hashes[0].back()+1);
        block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
    }
    std::sort(block.data_value_hashes[0].begin(), block.data_value_hashes[0].end());
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockExceedPreviousEpochSize) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    for(uint32_t i=0;i<1050;i++) {
        block.data_value_hashes[0].push_back(block.data_value_hashes[0].back()+1);
        block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
    }
    std::sort(block.data_value_hashes[0].begin(), block.data_value_hashes[0].end());
    block.data_value_hashes.emplace_back();
    block.mining_state.epoch++;
    block.mining_state.highest_hash_of_current_epoch = 0;
    block.mining_state.highest_hash_of_last_epoch = block.data_value_hashes[0].back();
    block.mining_state.num_minings_in_epoch = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockFitPreviousEpochSize) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    while(block.data_value_hashes[0].size() < 1000) {
        block.data_value_hashes[0].push_back(block.data_value_hashes[0].back()+1);
        block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
    }
    std::sort(block.data_value_hashes[0].begin(), block.data_value_hashes[0].end());
    block.data_value_hashes.emplace_back();
    block.mining_state.epoch++;
    block.mining_state.highest_hash_of_current_epoch = 0;
    block.mining_state.highest_hash_of_last_epoch = block.data_value_hashes[0].back();
    block.mining_state.num_minings_in_epoch = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_TRUE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockDeceedPreviousEpochSize) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.data_value_hashes.emplace_back();
    block.mining_state.epoch++;
    block.mining_state.highest_hash_of_current_epoch = 0;
    block.mining_state.highest_hash_of_last_epoch = block.data_value_hashes[0].back();
    block.mining_state.num_minings_in_epoch = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWrongEpoch) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.mining_state.epoch++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWrongHighestHashOfCurrentEpoch) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.mining_state.highest_hash_of_current_epoch++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWrongHighestHashOfLastEpoch) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    while(block.data_value_hashes[0].size() < 1000) {
        block.data_value_hashes[0].push_back(block.data_value_hashes[0].back()+1);
        block.wallets[example_owner_public_key] += TransactionSubBlock::fraction_per_coin;
    }
    std::sort(block.data_value_hashes[0].begin(), block.data_value_hashes[0].end());
    block.data_value_hashes.emplace_back();
    block.mining_state.epoch++;
    block.mining_state.highest_hash_of_current_epoch = 0;
    block.mining_state.highest_hash_of_last_epoch = 1234;
    block.mining_state.num_minings_in_epoch = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWrongNumMiningsInEpoch) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.mining_state.num_minings_in_epoch++;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWalletEmptyPublicKey) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    block.wallets[PublicKeyPEM("")] = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWalletInvalidPublicKey1) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    std::string invalid_key_string = other_public_key_2.getAsFullString();
    invalid_key_string[5]++;
    block.wallets[PublicKeyPEM(invalid_key_string)] = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWalletInvalidPublicKey2) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    std::string invalid_key_string = other_public_key_2.getAsFullString();
    invalid_key_string[50]++;
    block.wallets[PublicKeyPEM(invalid_key_string)] = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}

TEST_F(TestBlockchain, validateBaselineBlockWalletInvalidPublicKey3) {
    addBlock({valid_data_values_epoch_0[1]}, {}); //add block
    auto block = buildBaselineBlock(true, blockchain.getNewestBlock()->header.block_uid + 1, blockchain.getNewestBlock()->header.generic_header.block_hash);
    std::string invalid_key_string = other_public_key_2.getAsFullString();
    invalid_key_string.resize(invalid_key_string.length()-2);
    block.wallets[PublicKeyPEM(invalid_key_string)] = 0;
    block.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(block);
    EXPECT_FALSE(blockchain.validateBlock(block));
}
