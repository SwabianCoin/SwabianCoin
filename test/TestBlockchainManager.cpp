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

    TestBlockchainManager(bool synchronized = false) {

    }

    void init(bool synchronized, blockchain_time_t initial_time = (12892961ull + 1ull) * 120000ull + 1ull) {
        peer_stubs_ = std::make_unique<std::array<PeerStub, 10>>();
        sync_timer_stub_ = std::make_unique<SynchronizedTimerStub>(initial_time);
        blockchain_ = std::make_unique<Blockchain>("./blockchain_unit_test/");
        p2p_connector_stub_ = std::make_unique<P2PConnectorStub>();
        miner_ = std::make_unique<MinerLocal>(0);
        crypto_ = std::make_unique<CryptoHelper>(example_owner_public_key, example_owner_private_key);
        blockchain_manager_ = std::make_unique<BlockchainManager>(example_owner_public_key, example_owner_private_key, *blockchain_, *p2p_connector_stub_, *miner_, !synchronized, *sync_timer_stub_);

        for (uint32_t i = 0;i<peer_stubs_->size();i++) {
            (*peer_stubs_)[i].id_ = std::to_string(i);
            (*peer_stubs_)[i].info_ = "10.0.0.1" + std::to_string(i);
        }
        blockchain_->initEmptyChain();
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

    std::unique_ptr<std::array<PeerStub, 10>> peer_stubs_;
    std::unique_ptr<SynchronizedTimerStub> sync_timer_stub_;
    std::unique_ptr<Blockchain> blockchain_;
    std::unique_ptr<P2PConnectorStub> p2p_connector_stub_;
    std::unique_ptr<MinerLocal> miner_;
    std::unique_ptr<CryptoHelper> crypto_;
    std::unique_ptr<BlockchainManager> blockchain_manager_;


    void CheckBlockAcceptance(uint32_t check_time_in_cycle, bool accept, uint32_t sending_peers) {
        ASSERT_LE(sending_peers, 10);
        //wait one complete cycle (settling)
        sync_timer_stub_->letTheTimeGoOn(60000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sync_timer_stub_->letTheTimeGoOn(60000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //proceed to the first half of introduce block phase
        sync_timer_stub_->letTheTimeGoOn(30000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sync_timer_stub_->letTheTimeGoOn(check_time_in_cycle);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        //send a valid block containing a creation to blockchain manager
        auto previous_block = blockchain_->getNewestBlock();
        auto collection_block = std::make_shared<CollectionBlock>();
        collection_block->header.block_uid = previous_block->header.block_uid + 1;
        collection_block->header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        CreationSubBlock creation_sub_block;
        creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        creation_sub_block.creator = example_owner_public_key;
        creation_sub_block.data_value = valid_data_values_epoch_0[0];
        crypto_->fillSignature(creation_sub_block);
        CryptoHelper::fillHash(creation_sub_block);
        collection_block->creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
        CryptoHelper::fillHash(*collection_block);
        for(uint32_t i=0;i<sending_peers;i++) {
            p2p_connector_stub_->callback_collection_((*peer_stubs_)[i].getId(), collection_block, false);
        }

        //wait some time to let blockchain manager merge the creation
        sync_timer_stub_->letTheTimeGoOn(6000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        //check if blockchain manager merged the creation
        if(accept) {
            EXPECT_GE(p2p_connector_stub_->propagate_collection_block_counter_, 1);
            ASSERT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 1);
            EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.begin()->first,
                      creation_sub_block.header.generic_header.block_hash);
        } else {
            //check if blockchain manager merged the creation
            EXPECT_GE(p2p_connector_stub_->propagate_collection_block_counter_, 1);
            ASSERT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 0);
        }
    }
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

TEST_F(TestBlockchainManager, getPreviousBaselineBlockMethod) {
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(1), 1);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(2), 1);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(600), 1);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(720), 1);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(721), 721);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(722), 721);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(1440), 721);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(1441), 1441);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(1442), 1441);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(24480), 23761);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(24481), 24481);
    EXPECT_EQ(BlockchainManager::getPreviousBaselineBlock(24482), 24481);
}

TEST_F(TestBlockchainManager, InitialUnsynchronized) {
    init(false);
    EXPECT_EQ(blockchain_manager_->percentBlockchainSynchronized(), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->percentBlockchainSynchronized(), 0);
    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::FetchBlockchain);
}

TEST_F(TestBlockchainManager, Synchronization) {
    init(false, (12892961ull + 10ull) * 120000ull + 1ull); //let time jump to cycle 10
    //pregenerate simple blockchain
    auto remote_blockchain = std::make_shared<Blockchain>("./remote_blockchain/");
    {
        BaselineBlock root_block;
        root_block.header.block_uid = 1;
        root_block.data_value_hashes.resize(1);
        CryptoHelper::fillHash(root_block);
        remote_blockchain->setRootBlock(root_block);
        std::vector<CollectionBlock> collection_blocks;
        for (uint32_t i = 0; i < 9; i++) {
            collection_blocks.emplace_back();
            collection_blocks.back().header.block_uid = i + 2;
            collection_blocks.back().header.generic_header.previous_block_hash = (i == 0)
                                                                                 ? root_block.header.generic_header.block_hash
                                                                                 : collection_blocks[i - 1].header.generic_header.block_hash;
            CryptoHelper::fillHash(collection_blocks.back());
            remote_blockchain->addBlock(collection_blocks.back());
        }
    }

    EXPECT_LT(blockchain_manager_->percentBlockchainSynchronized(), 100);

    p2p_connector_stub_->setRemoteBlockchain((*peer_stubs_)[0].getId(), remote_blockchain);

    //let some time go by (3/4 cycle)
    for (auto i = 0; i < 6; i++) {
        sync_timer_stub_->letTheTimeGoOn(15000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    ASSERT_EQ(remote_blockchain->getRootBlockId(), blockchain_->getRootBlockId());
    ASSERT_EQ(remote_blockchain->getNewestBlockId(), blockchain_->getNewestBlockId());
    for(auto block_id = blockchain_->getRootBlockId();block_id <= blockchain_->getNewestBlockId();block_id++) {
        auto block = blockchain_->getBlock(block_id);
        auto remote_block = remote_blockchain->getBlock(block_id);
        EXPECT_EQ(block->header.generic_header.block_hash, remote_block->header.generic_header.block_hash);
    }
    EXPECT_EQ(blockchain_manager_->percentBlockchainSynchronized(), 100);
}

TEST_F(TestBlockchainManager, SynchronizationLong) {
    init(false, (12892961ull + 80ull) * 120000ull + 1ull); //let time jump to cycle 80
    //pregenerate simple blockchain
    auto remote_blockchain = std::make_shared<Blockchain>("./remote_blockchain/");
    {
        BaselineBlock root_block;
        root_block.header.block_uid = 1;
        root_block.data_value_hashes.resize(1);
        CryptoHelper::fillHash(root_block);
        remote_blockchain->setRootBlock(root_block);
        std::vector<CollectionBlock> collection_blocks;
        for (uint32_t i = 0; i < 80; i++) {
            collection_blocks.emplace_back();
            collection_blocks.back().header.block_uid = i + 2;
            collection_blocks.back().header.generic_header.previous_block_hash = (i == 0)
                                                                                 ? root_block.header.generic_header.block_hash
                                                                                 : collection_blocks[i - 1].header.generic_header.block_hash;
            CryptoHelper::fillHash(collection_blocks.back());
            remote_blockchain->addBlock(collection_blocks.back());
        }
    }

    EXPECT_LT(blockchain_manager_->percentBlockchainSynchronized(), 100);

    p2p_connector_stub_->setRemoteBlockchain((*peer_stubs_)[0].getId(), remote_blockchain);

    //let some time go by (3/4 cycle)
    for (auto i = 0; i < 6; i++) {
        sync_timer_stub_->letTheTimeGoOn(15000);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    ASSERT_EQ(remote_blockchain->getRootBlockId(), blockchain_->getRootBlockId());
    ASSERT_EQ(remote_blockchain->getNewestBlockId(), blockchain_->getNewestBlockId());
    for(auto block_id = blockchain_->getRootBlockId();block_id <= blockchain_->getNewestBlockId();block_id++) {
        auto block = blockchain_->getBlock(block_id);
        auto remote_block = remote_blockchain->getBlock(block_id);
        EXPECT_EQ(block->header.generic_header.block_hash, remote_block->header.generic_header.block_hash);
    }
    EXPECT_EQ(blockchain_manager_->percentBlockchainSynchronized(), 100);
}

TEST_F(TestBlockchainManager, CycleStateChanges) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::Collect);

    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::IntroduceBlock);

    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::Collect);
}

TEST_F(TestBlockchainManager, MergeBlocks) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    p2p_connector_stub_->propagate_collection_block_counter_ = 0;

    //proceed from collect phase to introduce block phase
    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if initial block has been sent by blockchain manager
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 1);
    EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 0);

    //send a valid block containing a creation to blockchain manager
    auto previous_block = blockchain_->getNewestBlock();
    auto collection_block = std::make_shared<CollectionBlock>();
    collection_block->header.block_uid = previous_block->header.block_uid + 1;
    collection_block->header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    CreationSubBlock creation_sub_block;
    creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    creation_sub_block.creator = example_owner_public_key;
    creation_sub_block.data_value = valid_data_values_epoch_0[0];
    crypto_->fillSignature(creation_sub_block);
    CryptoHelper::fillHash(creation_sub_block);
    collection_block->creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
    CryptoHelper::fillHash(*collection_block);
    p2p_connector_stub_->callback_collection_((*peer_stubs_)[0].getId(), collection_block, false);

    //wait some time to let blockchain manager merge the creation
    sync_timer_stub_->letTheTimeGoOn(6000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if blockchain manager merged the creation
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 2);
    ASSERT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 1);
    EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.begin()->first,creation_sub_block.header.generic_header.block_hash);
}

TEST_F(TestBlockchainManager, MergeBlocksDuplicate) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    p2p_connector_stub_->propagate_collection_block_counter_ = 0;

    //proceed from collect phase to introduce block phase
    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if initial block has been sent by blockchain manager
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 1);
    EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 0);

    //send a valid block containing two creations with different hashes but same data value to blockchain manager
    auto previous_block = blockchain_->getNewestBlock();
    auto collection_block = std::make_shared<CollectionBlock>();
    collection_block->header.block_uid = previous_block->header.block_uid + 1;
    collection_block->header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    CreationSubBlock creation_sub_block_a, creation_sub_block_b;
    {
        creation_sub_block_a.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        creation_sub_block_a.creator = example_owner_public_key;
        creation_sub_block_a.data_value = valid_data_values_epoch_0[0];
        crypto_->fillSignature(creation_sub_block_a);
        CryptoHelper::fillHash(creation_sub_block_a);
        collection_block->creations[creation_sub_block_a.header.generic_header.block_hash] = creation_sub_block_a;
    }
    {
        creation_sub_block_b.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
        creation_sub_block_b.creator = example_owner_public_key;
        creation_sub_block_b.data_value = valid_data_values_epoch_0[0];
        crypto_->fillSignature(creation_sub_block_b);
        CryptoHelper::fillHash(creation_sub_block_b);
        collection_block->creations[creation_sub_block_b.header.generic_header.block_hash] = creation_sub_block_b;
    }
    CryptoHelper::fillHash(*collection_block);
    p2p_connector_stub_->callback_collection_((*peer_stubs_)[0].getId(), collection_block, false);

    //wait some time to let blockchain manager merge the creation
    sync_timer_stub_->letTheTimeGoOn(6000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if blockchain manager merged the creation twice
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 2);
    ASSERT_LT(p2p_connector_stub_->last_collection_block_.creations.size(), 2);
}

TEST_F(TestBlockchainManager, MergeBlocksDuplicate2) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    p2p_connector_stub_->propagate_collection_block_counter_ = 0;

    //proceed from collect phase to introduce block phase
    sync_timer_stub_->letTheTimeGoOn(30000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if initial block has been sent by blockchain manager
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 1);
    EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 0);

    //send a valid block containing a creation to blockchain manager
    auto previous_block = blockchain_->getNewestBlock();
    auto collection_block = std::make_shared<CollectionBlock>();
    collection_block->header.block_uid = previous_block->header.block_uid + 1;
    collection_block->header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    CreationSubBlock creation_sub_block;
    creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    creation_sub_block.creator = example_owner_public_key;
    creation_sub_block.data_value = valid_data_values_epoch_0[0];
    crypto_->fillSignature(creation_sub_block);
    CryptoHelper::fillHash(creation_sub_block);
    collection_block->creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
    std::cout << "Hash 1: " << hash_helper::toString(creation_sub_block.header.generic_header.block_hash) << std::endl;
    CryptoHelper::fillHash(*collection_block);
    p2p_connector_stub_->callback_collection_((*peer_stubs_)[0].getId(), collection_block, false);

    sync_timer_stub_->letTheTimeGoOn(4000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //send a valid block containing the same creation but a different hash to blockchain manager
    collection_block->creations.clear();
    collection_block->header.generic_header.block_hash = 0;
    creation_sub_block.header.generic_header.block_hash = 0;
    creation_sub_block.signature = "";
    collection_block->header.block_uid = previous_block->header.block_uid + 1;
    collection_block->header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    creation_sub_block.header.generic_header.previous_block_hash = previous_block->header.generic_header.block_hash;
    creation_sub_block.creator = example_owner_public_key;
    creation_sub_block.data_value = valid_data_values_epoch_0[0];
    crypto_->fillSignature(creation_sub_block);
    CryptoHelper::fillHash(creation_sub_block);
    collection_block->creations[creation_sub_block.header.generic_header.block_hash] = creation_sub_block;
    std::cout << "Hash 2: " << hash_helper::toString(creation_sub_block.header.generic_header.block_hash) << std::endl;
    CryptoHelper::fillHash(*collection_block);
    p2p_connector_stub_->callback_collection_((*peer_stubs_)[0].getId(), collection_block, false);

    //wait some time to let blockchain manager merge the creation
    sync_timer_stub_->letTheTimeGoOn(6000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //check if blockchain manager merged the creation only once
    EXPECT_EQ(p2p_connector_stub_->propagate_collection_block_counter_, 3);
    ASSERT_EQ(p2p_connector_stub_->last_collection_block_.creations.size(), 1);
    EXPECT_EQ(p2p_connector_stub_->last_collection_block_.creations.begin()->second.data_value,creation_sub_block.data_value);
}


TEST_F(TestBlockchainManager, AcceptEarlyBlock) {
    init(true);
    CheckBlockAcceptance(0, true, 1);
}

TEST_F(TestBlockchainManager, AcceptInTimeBlock) {
    init(true);
    CheckBlockAcceptance(35000, true, 1);
}

TEST_F(TestBlockchainManager, RefuseOutOfTimeBlock1) {
    init(true);
    CheckBlockAcceptance(60000, false, 1);
}

TEST_F(TestBlockchainManager, RefuseOutOfTimeBlock2) {
    init(true);
    CheckBlockAcceptance(75000, false, 1);
}

TEST_F(TestBlockchainManager, AcceptLateBlockMultiplePeers1) {
    init(true);
    CheckBlockAcceptance(60000, true, 5);
}

TEST_F(TestBlockchainManager, AcceptLateBlockMultiplePeers2) {
    init(true);
    CheckBlockAcceptance(75000, true, 7);
}


TEST_F(TestBlockchainManager, OutOfSyncDetectionGoodWeather) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::Collect);

    //wait one complete cycle
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::Collect);
}

TEST_F(TestBlockchainManager, OutOfSyncDetectionBadWeather1) {
    init(true);
    //wait one complete cycle (settling)
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::Collect);

    sync_timer_stub_->letTheTimeGoOn(120000); //fast forward some cycles

    //wait one complete cycle
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync_timer_stub_->letTheTimeGoOn(60000);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(blockchain_manager_->getCurrentState(), ICycleState::State::FetchBlockchain);
}
