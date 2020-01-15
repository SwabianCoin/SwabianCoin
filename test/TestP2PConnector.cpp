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

#include "scn/Blockchain/Blockchain.h"
#include "scn/P2PConnector/P2PConnector.h"
#include "PeerStub.h"
#include <gtest/gtest.h>

using namespace scn;


class TestP2PConnector : public testing::Test {
public:
    TestP2PConnector()
    :blockchain_("./blockchain_test/")
    ,p2p_connector_(13386, blockchain_)
    ,last_received_baseline_block(nullptr)
    ,last_received_collection_block(nullptr) {
        p2p_connector_.registerBlockCallbacks(std::bind(&TestP2PConnector::baselineBlockReceivedCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                              std::bind(&TestP2PConnector::collectionBlockReceivedCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        p2p_connector_.connect();
    }

    void createSimpleBaselineBlock(BaselineBlock& block, std::string& serialized_block) {
        //create block
        block.header.block_uid = 721;
        block.header.generic_header.previous_block_hash = 12345;
        block.data_value_hashes.resize(3);
        block.data_value_hashes[1].resize(18);
        block.data_value_hashes[1][2] = 45678;
        CryptoHelper::fillHash(block);

        //serialize block
        std::stringstream oss;
        cereal::PortableBinaryOutputArchive oa(oss);
        oa << (uint16_t)1; //protocol version
        oa << (uint8_t)1; //PropagateBaselineBlock
        oa << block;
        oa << true; //reply
        serialized_block = oss.str();
    }

    void createSimpleCollectionBlock(CollectionBlock& block, std::string& serialized_block) {
        //create block
        block.header.block_uid = 721;
        block.header.generic_header.previous_block_hash = 12345;
        block.creations[123].data_value = "abc";
        block.transactions[456].post_owner = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----");
        CryptoHelper::fillHash(block);

        //serialize block
        std::stringstream oss;
        cereal::PortableBinaryOutputArchive oa(oss);
        oa << (uint16_t)1; //protocol version
        oa << (uint8_t)2; //PropagateCollectionBlock
        oa << block;
        oa << true; //reply
        serialized_block = oss.str();
    }

protected:
    void baselineBlockReceivedCallback(IPeer& peer, const BaselineBlock& block, bool reply) {
        last_received_baseline_block = std::make_shared<BaselineBlock>(block);
    }

    void collectionBlockReceivedCallback(IPeer& peer, const CollectionBlock& block, bool reply) {
        last_received_collection_block = std::make_shared<CollectionBlock>(block);
    }

    PeerStub dummy_peer_;
    Blockchain blockchain_;
    P2PConnector p2p_connector_;

    std::shared_ptr<BaselineBlock> last_received_baseline_block;
    std::shared_ptr<CollectionBlock> last_received_collection_block;
};


TEST_F(TestP2PConnector, receiveValidBaselineBlock) {
    BaselineBlock block;
    std::string serialized_block;
    createSimpleBaselineBlock(block, serialized_block);

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if p2p connector passed block to observer
    EXPECT_NE(last_received_baseline_block, nullptr);
    EXPECT_EQ(last_received_baseline_block->header.block_uid, 721);
    EXPECT_EQ(last_received_baseline_block->header.generic_header.previous_block_hash, 12345);
    EXPECT_EQ(last_received_baseline_block->data_value_hashes.size(), 3);
    EXPECT_EQ(last_received_baseline_block->data_value_hashes[1].size(), 18);
    EXPECT_EQ(last_received_baseline_block->data_value_hashes[1][2], 45678);
    EXPECT_EQ(last_received_baseline_block->header.generic_header.block_hash, block.header.generic_header.block_hash);
}

TEST_F(TestP2PConnector, receiveInvalidBaselineBlock1) {
    BaselineBlock block;
    std::string serialized_block;
    createSimpleBaselineBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block.substr(0, serialized_block.length()-2);

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if p2p connector passed block to observer
    EXPECT_EQ(last_received_baseline_block, nullptr);
}

TEST_F(TestP2PConnector, receiveInvalidBaselineBlock2) {
    BaselineBlock block;
    std::string serialized_block;
    createSimpleBaselineBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[20]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_baseline_block == nullptr || (
            last_received_baseline_block->header.block_uid != 721 ||
            last_received_baseline_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_baseline_block->data_value_hashes.size() != 3 ||
            last_received_baseline_block->data_value_hashes[1].size() != 18 ||
            last_received_baseline_block->data_value_hashes[1][2] != 45678 ||
            last_received_baseline_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}

TEST_F(TestP2PConnector, receiveInvalidBaselineBlock3) {
    BaselineBlock block;
    std::string serialized_block;
    createSimpleBaselineBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[0]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_baseline_block == nullptr || (
            last_received_baseline_block->header.block_uid != 721 ||
            last_received_baseline_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_baseline_block->data_value_hashes.size() != 3 ||
            last_received_baseline_block->data_value_hashes[1].size() != 18 ||
            last_received_baseline_block->data_value_hashes[1][2] != 45678 ||
            last_received_baseline_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}

TEST_F(TestP2PConnector, receiveInvalidBaselineBlock4) {
    BaselineBlock block;
    std::string serialized_block;
    createSimpleBaselineBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[3]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_baseline_block == nullptr || (
            last_received_baseline_block->header.block_uid != 721 ||
            last_received_baseline_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_baseline_block->data_value_hashes.size() != 3 ||
            last_received_baseline_block->data_value_hashes[1].size() != 18 ||
            last_received_baseline_block->data_value_hashes[1][2] != 45678 ||
            last_received_baseline_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}


TEST_F(TestP2PConnector, receiveValidCollectionBlock) {
    CollectionBlock block;
    std::string serialized_block;
    createSimpleCollectionBlock(block, serialized_block);

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if p2p connector passed block to observer
    EXPECT_NE(last_received_collection_block, nullptr);
    EXPECT_EQ(last_received_collection_block->header.block_uid, 721);
    EXPECT_EQ(last_received_collection_block->header.generic_header.previous_block_hash, 12345);
    EXPECT_EQ(last_received_collection_block->creations[123].data_value, "abc");
    EXPECT_EQ(last_received_collection_block->transactions[456].post_owner, PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----"));
    EXPECT_EQ(last_received_collection_block->header.generic_header.block_hash, block.header.generic_header.block_hash);
}

TEST_F(TestP2PConnector, receiveInvalidCollectionBlock1) {
    CollectionBlock block;
    std::string serialized_block;
    createSimpleCollectionBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block.substr(0, serialized_block.length()-2);

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_collection_block == nullptr || (
            last_received_collection_block->header.block_uid != 721 ||
            last_received_collection_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_collection_block->creations[123].data_value != "abc" ||
            last_received_collection_block->transactions[456].post_owner != PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----") ||
            last_received_collection_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}

TEST_F(TestP2PConnector, receiveInvalidCollectionBlock2) {
    CollectionBlock block;
    std::string serialized_block;
    createSimpleCollectionBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[20]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_collection_block == nullptr || (
            last_received_collection_block->header.block_uid != 721 ||
            last_received_collection_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_collection_block->creations[123].data_value != "abc" ||
            last_received_collection_block->transactions[456].post_owner != PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----") ||
            last_received_collection_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}

TEST_F(TestP2PConnector, receiveInvalidCollectionBlock3) {
    CollectionBlock block;
    std::string serialized_block;
    createSimpleCollectionBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[0]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_collection_block == nullptr || (
            last_received_collection_block->header.block_uid != 721 ||
            last_received_collection_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_collection_block->creations[123].data_value != "abc" ||
            last_received_collection_block->transactions[456].post_owner != PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----") ||
            last_received_collection_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}

TEST_F(TestP2PConnector, receiveInvalidCollectionBlock4) {
    CollectionBlock block;
    std::string serialized_block;
    createSimpleCollectionBlock(block, serialized_block);

    std::string modified_serialized_block = serialized_block;
    modified_serialized_block[3]++;

    //give serialized block to p2p connector
    p2p_connector_.receivedMessage(dummy_peer_, modified_serialized_block);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_TRUE(last_received_collection_block == nullptr || (
            last_received_collection_block->header.block_uid != 721 ||
            last_received_collection_block->header.generic_header.previous_block_hash != 12345 ||
            last_received_collection_block->creations[123].data_value != "abc" ||
            last_received_collection_block->transactions[456].post_owner != PublicKeyPEM("-----BEGIN PUBLIC KEY-----\ndef\n-----END PUBLIC KEY-----") ||
            last_received_collection_block->header.generic_header.block_hash != block.header.generic_header.block_hash));
}


TEST_F(TestP2PConnector, receiveNonsense) {

    std::string nonsense_string = "0123546 asfioj";

    //give serialized block to p2p connector
    EXPECT_NO_FATAL_FAILURE(p2p_connector_.receivedMessage(dummy_peer_, nonsense_string));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
