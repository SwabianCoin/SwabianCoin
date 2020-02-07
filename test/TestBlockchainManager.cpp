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

    TestBlockchainManager(bool synchronized = false)
    :peer_stubs_()
    ,sync_timer_stub_(7*120000 + 1) //begin at cycle start
    ,blockchain_("./blockchain_unit_test/")
    ,p2p_connector_stub_()
    ,miner_(0)
    ,crypto_(example_owner_public_key, example_owner_private_key)
    ,blockchain_manager_(example_owner_public_key, example_owner_private_key, blockchain_, p2p_connector_stub_, miner_, !synchronized, sync_timer_stub_) {
        for(uint32_t i=0;i<peer_stubs_.size();i++) {
            peer_stubs_[i].id_ = std::to_string(i);
            peer_stubs_[i].info_ = "10.0.0.1" + std::to_string(i);
        }
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

    std::vector<std::string> valid_data_values_epoch_0 = {
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_7B",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_618",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_6D7",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_947",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_95A",
            "0_MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==_248833349_A52"
    };

    std::array<PeerStub, 10> peer_stubs_;
    SynchronizedTimerStub sync_timer_stub_;
    Blockchain blockchain_;
    P2PConnectorStub p2p_connector_stub_;
    MinerLocal miner_;
    CryptoHelper crypto_;
    BlockchainManager blockchain_manager_;


    void CheckBlockAcceptance(uint32_t check_time_in_cycle, bool accept, uint32_t sending_peers) {
        ASSERT_LE(sending_peers, 10);
        //wait one complete cycle (settling)
        sync_timer_stub_.letTheTimeGoOn(60000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sync_timer_stub_.letTheTimeGoOn(60000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //proceed to the first half of introduce block phase
        sync_timer_stub_.letTheTimeGoOn(30000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sync_timer_stub_.letTheTimeGoOn(check_time_in_cycle);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        //send a valid block containing a creation to blockchain manager
        auto previous_block = blockchain_.getNewestBlock();
        CollectionBlock collection_block;
        collection_block.header.block_uid = previous_block->header.block_uid + 1;
        collection_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        CreationSubBlock creation_sub_block;
        creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        creation_sub_block.creator = example_owner_public_key;
        creation_sub_block.data_value = valid_data_values_epoch_0[0];
        crypto_.fillSignature(creation_sub_block);
        CryptoHelper::fillHash(creation_sub_block);
        collection_block.creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
        CryptoHelper::fillHash(collection_block);
        for(uint32_t i=0;i<sending_peers;i++) {
            p2p_connector_stub_.callback_collection_(peer_stubs_[i], collection_block, false);
        }

        //wait some time to let blockchain manager merge the creation
        sync_timer_stub_.letTheTimeGoOn(6000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //check if blockchain manager merged the creation
        if(accept) {
            EXPECT_GE(p2p_connector_stub_.propagate_collection_block_counter_, 1);
            ASSERT_EQ(p2p_connector_stub_.last_collection_block_.creations.size(), 1);
            EXPECT_EQ(p2p_connector_stub_.last_collection_block_.creations.begin()->first,
                      creation_sub_block.header.generic_header.block_hash);
        } else {
            //check if blockchain manager merged the creation
            EXPECT_GE(p2p_connector_stub_.propagate_collection_block_counter_, 1);
            ASSERT_EQ(p2p_connector_stub_.last_collection_block_.creations.size(), 0);
        }
    }
};

class TestBlockchainManagerSynchronized : public TestBlockchainManager {
public:
    TestBlockchainManagerSynchronized()
    :TestBlockchainManager(true) {}
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

    EXPECT_EQ(blockchain_manager_.percentBlockchainSynchronized(), 0);
    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::FetchBlockchain);
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
    p2p_connector_stub_.callback_baseline_(peer_stubs_[0], root_block, true);

    for(uint32_t i=0;i<collection_blocks.size();i++) {
        //let some time go by
        sync_timer_stub_.letTheTimeGoOn(200000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //check if manager asked for follow up block
        EXPECT_GE(p2p_connector_stub_.ask_for_block_counter_, 1);
        EXPECT_EQ(p2p_connector_stub_.ask_for_block_uid_, i+2);

        //send follow up block
        p2p_connector_stub_.callback_collection_(peer_stubs_[0], collection_blocks[i], true);
    }
}

TEST_F(TestBlockchainManagerSynchronized, CycleStateChanges) {
    //wait one complete cycle (settling)
    sync_timer_stub_.letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_.letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::Collect);

    sync_timer_stub_.letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_.letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_.letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_.letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_.getCurrentState(), ICycleState::State::Collect);
}

TEST_F(TestBlockchainManagerSynchronized, MergeBlocks) {
    //wait one complete cycle (settling)
    sync_timer_stub_.letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_.letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    p2p_connector_stub_.propagate_collection_block_counter_ = 0;

    //proceed from collect phase to introduce block phase
    sync_timer_stub_.letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if initial block has been sent by blockchain manager
    EXPECT_EQ(p2p_connector_stub_.propagate_collection_block_counter_, 1);
    EXPECT_EQ(p2p_connector_stub_.last_collection_block_.creations.size(), 0);

    //send a valid block containing a creation to blockchain manager
    auto previous_block = blockchain_.getNewestBlock();
    CollectionBlock collection_block;
    collection_block.header.block_uid = previous_block->header.block_uid + 1;
    collection_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    CreationSubBlock creation_sub_block;
    creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    creation_sub_block.creator = example_owner_public_key;
    creation_sub_block.data_value = valid_data_values_epoch_0[0];
    crypto_.fillSignature(creation_sub_block);
    CryptoHelper::fillHash(creation_sub_block);
    collection_block.creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
    CryptoHelper::fillHash(collection_block);
    p2p_connector_stub_.callback_collection_(peer_stubs_[0], collection_block, false);

    //wait some time to let blockchain manager merge the creation
    sync_timer_stub_.letTheTimeGoOn(6000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if blockchain manager merged the creation
    EXPECT_EQ(p2p_connector_stub_.propagate_collection_block_counter_, 2);
    ASSERT_EQ(p2p_connector_stub_.last_collection_block_.creations.size(), 1);
    EXPECT_EQ(p2p_connector_stub_.last_collection_block_.creations.begin()->first,creation_sub_block.header.generic_header.block_hash);
}



TEST_F(TestBlockchainManagerSynchronized, AcceptEarlyBlock) {
    CheckBlockAcceptance(0, true, 1);
}

TEST_F(TestBlockchainManagerSynchronized, AcceptInTimeBlock) {
    CheckBlockAcceptance(35000, true, 1);
}

TEST_F(TestBlockchainManagerSynchronized, RefuseOutOfTimeBlock1) {
    CheckBlockAcceptance(60000, false, 1);
}

TEST_F(TestBlockchainManagerSynchronized, RefuseOutOfTimeBlock2) {
    CheckBlockAcceptance(75000, false, 1);
}

TEST_F(TestBlockchainManagerSynchronized, AcceptLateBlockMultiplePeers1) {
    CheckBlockAcceptance(60000, true, 5);
}

TEST_F(TestBlockchainManagerSynchronized, AcceptLateBlockMultiplePeers2) {
    CheckBlockAcceptance(75000, true, 7);
}