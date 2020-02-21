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

#include <fstream>
#include "CycleStateCollect.h"
#include "BlockchainManager.h"
#include "scn/Blockchain/Blockchain.h"

using namespace scn;

CycleStateCollect::CycleStateCollect(BlockchainManager& base)
:base_(base) {
}


CycleStateCollect::~CycleStateCollect() = default;


void CycleStateCollect::onEnter() {
    LOG(INFO) << "enter collect state - num_peers: " << base_.p2p_connector_.numConnectedPeers();

    base_.new_block_ = CollectionBlock();
    base_.new_block_.transactions.clear();
    base_.new_block_.creations.clear();
    minings_in_cycle_ = 0;
    max_cycle_duration_ms_ = 0;

    if(base_.blockchain_.getNewestBlockId() % ActivePeersCollector::blocks_to_collect_active_peers_ == 0) {
        base_.active_peers_collector_.restartListBuilding();
    }
    base_.active_peers_collector_.propagate();
}


bool CycleStateCollect::onCycle() {
    bool no_sleep = false;
    auto start = std::chrono::system_clock::now();
    
    auto newest_block_in_chain = base_.blockchain_.getNewestBlock();
    auto mining_state = base_.blockchain_.getMiningState();
    {
        std::string creation_data_value;
        {
            LOCK_MUTEX_WATCHDOG(base_.mtx_found_hash_queue_access_);
            if (!base_.found_hash_queue_.empty()) {
                creation_data_value = base_.found_hash_queue_.front();
                base_.found_hash_queue_.pop();
                if(!base_.found_hash_queue_.empty()) {
                    no_sleep = true;
                }
            }
        }
        if (creation_data_value.length() > 0) {
            CreationSubBlock sub_block;
            sub_block.header.generic_header.previous_block_hash = newest_block_in_chain->header.generic_header.block_hash;
            sub_block.data_value = creation_data_value;
            sub_block.creator = base_.our_public_key_;
            base_.crypto_.fillSignature(sub_block);
            CryptoHelper::fillHash(sub_block);
            if (base_.blockchain_.validateSubBlock(sub_block)) {
                base_.new_block_.creations[sub_block.header.generic_header.block_hash] = sub_block;
                minings_in_cycle_++;
                if(base_.new_block_.creations.size() > (CollectionBlock::max_num_creations - mining_state.num_minings_in_epoch)) {
                    base_.new_block_.creations.erase(std::prev(base_.new_block_.creations.end()));
                    minings_in_cycle_--;
                }
            } else {
                LOG(ERROR) << "self check validateSubBlock creation failed!";
            }
        }
    }

    {
        public_key_t transaction_post_owner;
        uint64_t transaction_fraction = 0;
        {
            LOCK_MUTEX_WATCHDOG(base_.mtx_transaction_queue_access_);
            if (!base_.transaction_queue_.empty()) {
                transaction_post_owner = base_.transaction_queue_.front().first;;
                transaction_fraction = base_.transaction_queue_.front().second;
                base_.transaction_queue_.pop();
                if(!base_.transaction_queue_.empty()) {
                    no_sleep = true;
                }
            }
        }
        if (!transaction_post_owner.isEmpty()) {
            TransactionSubBlock sub_block;
            sub_block.header.generic_header.previous_block_hash = newest_block_in_chain->header.generic_header.block_hash;
            sub_block.pre_owner = base_.our_public_key_;
            sub_block.post_owner = transaction_post_owner;
            sub_block.fraction = transaction_fraction;
            base_.crypto_.fillSignature(sub_block);
            CryptoHelper::fillHash(sub_block);
            if (base_.blockchain_.validateSubBlock(sub_block)) {
                base_.new_block_.transactions[sub_block.header.generic_header.block_hash] = sub_block;
                if(base_.new_block_.transactions.size() > CollectionBlock::max_num_transactions) {
                    base_.new_block_.transactions.erase(std::prev(base_.new_block_.transactions.end()));
                }
            } else {
                LOG(ERROR) << "self check validateSubBlock transaction failed!";
            }
        }
    }

    auto end = std::chrono::system_clock::now();
    uint32_t cycle_duration = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
    if(cycle_duration > max_cycle_duration_ms_) {
        max_cycle_duration_ms_ = cycle_duration;
    }

    return no_sleep;
}


void CycleStateCollect::onExit() {
    LOG(INFO) << "Minings in this collect cycle: " << minings_in_cycle_;
    LOG(INFO) << "Max collect cycle duration: " << max_cycle_duration_ms_ << " ms";
}
