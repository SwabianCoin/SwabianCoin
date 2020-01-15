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

#include "CycleStateIntroduceBlock.h"
#include "BlockchainManager.h"
#include <algorithm>

using namespace scn;

CycleStateIntroduceBlock::CycleStateIntroduceBlock(BlockchainManager& base)
:base_(base) {

}


CycleStateIntroduceBlock::~CycleStateIntroduceBlock() {

}


void CycleStateIntroduceBlock::onEnter() {
    LOG(INFO) << "enter introduce block state - num_peers: " << base_.p2p_connector_.numConnectedPeers();

    base_.out_of_sync_detector_.restartCheckCycle(base_.blockchain_);

    processed_blocks_.clear();

    block_uid_t new_block_uid = base_.blockchain_.getNewestBlockId() + 1;
    base_.new_block_.header.generic_header.previous_block_hash = base_.blockchain_.getBlock(new_block_uid-1)->header.generic_header.block_hash;
    base_.new_block_.header.block_uid = new_block_uid;
    base_.new_block_.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(base_.new_block_);

    LOG(INFO) << "Propagate initial block: " << base_.new_block_.header.generic_header.block_hash.str(0, std::ios_base::hex);
    base_.p2p_connector_.propagateBlock(base_.new_block_);
    next_propagation_time_ = base_.sync_timer_.now() + time_between_propagations_ms_;
}


bool CycleStateIntroduceBlock::onCycle() {
    if(base_.sync_timer_.now() >= next_propagation_time_) {
        next_propagation_time_ += time_between_propagations_ms_;
        LOG(INFO) << "Propagate updated block: " << base_.new_block_.header.generic_header.block_hash.str(0, std::ios_base::hex);
        base_.p2p_connector_.propagateBlock(base_.new_block_);
    }
    return false;
}


void CycleStateIntroduceBlock::onExit() {
    base_.new_block_.header.generic_header.block_hash = 0;
    CryptoHelper::fillHash(base_.new_block_);
    base_.blockchain_.addBlock(base_.new_block_);

    auto mining_state = base_.blockchain_.getMiningState();
    if(mining_state.epoch != base_.miner_.getEpoch() && base_.miner_.isRunning()) {
        base_.pauseMiner();
        {
            LOCK_MUTEX_WATCHDOG(base_.mtx_found_hash_queue_access_);
            std::queue<std::string>().swap(base_.found_hash_queue_); //clear
        }
        base_.resumeMiner();
    }

    LOG(INFO) << "New Block " << base_.new_block_.header.block_uid << ": " << base_.new_block_.header.generic_header.block_hash.str(0, std::ios_base::hex);
    LOG(INFO) << "      highest_hash_of_last_epoch: " << mining_state.highest_hash_of_last_epoch.str(0, std::ios_base::hex);
    LOG(INFO) << "Balance: " << static_cast<double>(base_.blockchain_.getBalance(base_.our_public_key_)) /
            static_cast<double>(TransactionSubBlock::fraction_per_coin);
}


void CycleStateIntroduceBlock::blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) {
    if(reply) {
        LOG(ERROR) << "received unexpected reply";
        base_.peers_monitor_.reportViolation(peer);
        return;
    }
    LOG(INFO) << "CycleStateIntroduceBlock incoming block: " << block.header.generic_header.block_hash.str(0, std::ios_base::hex);
    if(block.header.block_uid != base_.new_block_.header.block_uid) {
        LOG(INFO) << "  Ignoring block (unexpected block id " << block.header.block_uid << ")";
    } else if(processed_blocks_.find(block.header.generic_header.block_hash) != processed_blocks_.end()) {
        LOG(INFO) << "  Ignoring block (already processed)";
    } else {
        processed_blocks_.insert(block.header.generic_header.block_hash);
        std::chrono::time_point<std::chrono::system_clock> t0, t1, t2, t3, t4, t5;
        t0 = std::chrono::system_clock::now();
        if (base_.blockchain_.validateBlock(block)) {
            t1 = std::chrono::system_clock::now();
            auto mining_state = base_.blockchain_.getMiningState();
            t2 = std::chrono::system_clock::now();
            if (block.header.generic_header.block_hash != base_.new_block_.header.generic_header.block_hash) {
                //transactions
                for (auto &element : block.transactions) {
                    if (base_.new_block_.transactions.find(element.first) == base_.new_block_.transactions.end()) {
                        base_.new_block_.transactions[element.first] = element.second;
                        if (base_.new_block_.transactions.size() > CollectionBlock::max_num_transactions) {
                            base_.new_block_.transactions.erase(std::prev(base_.new_block_.transactions.end()));
                        }
                    }
                }
                t3 = std::chrono::system_clock::now();
                //creations
                for (auto &element : block.creations) {
                    if (base_.new_block_.creations.find(element.first) == base_.new_block_.creations.end()) {
                        base_.new_block_.creations[element.first] = element.second;
                        if (base_.new_block_.creations.size() >
                            (CollectionBlock::max_num_creations - mining_state.num_minings_in_epoch)) {
                            base_.new_block_.creations.erase(std::prev(base_.new_block_.creations.end()));
                        }
                    }
                }
                t4 = std::chrono::system_clock::now();
                base_.new_block_.header.generic_header.block_hash = 0;
                CryptoHelper::fillHash(base_.new_block_);
                t5 = std::chrono::system_clock::now();

                LOG(INFO) << "CycleStateIntroduceBlock::blockReceivedCallback durations t01:" <<
                          (std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)).count() << " t12:" <<
                          (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count() << " t23:" <<
                          (std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2)).count() << " t34:" <<
                          (std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3)).count() << " t45:" <<
                          (std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4)).count();
            }
        } else {
            LOG(ERROR) << "CycleStateIntroduceBlock incoming block invalid!";
            base_.peers_monitor_.reportViolation(peer);
        }
    }

    base_.out_of_sync_detector_.blockReceivedCallback(peer, block, reply);
}
