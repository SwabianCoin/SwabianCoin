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

#include "scn/SynchronizedTime/SynchronizedTimerStub.h"
#include "scn/Blockchain/Blockchain.h"
#include "scn/P2PConnector/P2PConnectorStub.h"
#include "scn/Miner/MinerLocal.h"
#include "scn/BlockchainManager/BlockchainManager.h"
#include "PeerStub.h"
#include <gtest/gtest.h>

using namespace scn;


class TestBlockchainManager : public testing::Test {
public:

    TestBlockchainManager()
    :peer_stub_()
    ,sync_timer_stub_()
    ,blockchain_("./blockchain_unit_test/")
    ,p2p_connector_stub_()
    ,miner_(1)
    ,blockchain_manager_(example_owner_public_key, example_owner_private_key, blockchain_, p2p_connector_stub_, miner_, true, sync_timer_stub_) {
        blockchain_.initEmptyChain();
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

    PeerStub peer_stub_;
    SynchronizedTimerStub sync_timer_stub_;
    Blockchain blockchain_;
    P2PConnectorStub p2p_connector_stub_;
    MinerLocal miner_;
    BlockchainManager blockchain_manager_;
};


TEST_F(TestBlockchainManager, isBaselineMethod) {
    EXPECT_EQ(BlockchainManager::isBaselineBlock(1), true);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(2), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(600), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(720), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(721), true);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(722), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(1440), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(1441), true);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(1442), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(24480), false);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(24481), true);
    EXPECT_EQ(BlockchainManager::isBaselineBlock(24482), false);
}

TEST_F(TestBlockchainManager, getNextBaselineBlockMethod) {
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(1), 721);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(2), 721);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(600), 721);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(720), 721);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(721), 1441);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(722), 1441);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(1440), 1441);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(1441), 2161);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(1442), 2161);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(24480), 24481);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(24481), 25201);
    EXPECT_EQ(BlockchainManager::getNextBaselineBlock(24482), 25201);
}

TEST_F(TestBlockchainManager, InitialUnsynchronized) {
    EXPECT_EQ(blockchain_manager_.percentBlockchainSynchronized(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST_F(TestBlockchainManager, Synchronization) {

    //pregenerate some blocks
    BaselineBlock root_block;
    root_block.header.block_uid = 1;
    root_block.data_value_hashes.resize(1);
    CryptoHelper::fillHash(root_block);
    std::vector<CollectionBlock> collection_blocks;
    for(uint32_t i=0;i<5;i++) {
        collection_blocks.emplace_back();
        collection_blocks.back().header.block_uid = i+2;
        collection_blocks.back().header.generic_header.previous_block_hash = (i==0) ? root_block.header.generic_header.block_hash : collection_blocks[i-1].header.generic_header.block_hash;
        CryptoHelper::fillHash(collection_blocks.back());
    }

    //let some time go by
    sync_timer_stub_.letTheTimeGoOn(200000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if manager asked for baseline block
    EXPECT_GE(p2p_connector_stub_.ask_for_last_baseline_block_counter_, 1);

    //send baseline block to manager
    p2p_connector_stub_.callback_baseline_(peer_stub_, root_block, true);

    for(uint32_t i=0;i<collection_blocks.size();i++) {
        //let some time go by
        sync_timer_stub_.letTheTimeGoOn(200000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //check if manager asked for follow up block
        EXPECT_GE(p2p_connector_stub_.ask_for_block_counter_, 1);
        EXPECT_EQ(p2p_connector_stub_.ask_for_block_uid_, i+2);

        //send follow up block
        p2p_connector_stub_.callback_collection_(peer_stub_, collection_blocks[i], true);
    }
}