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

#include "CycleStateFetchBlockchain.h"
#include "BlockchainManager.h"
#include "scn/Blockchain/Blockchain.h"

using namespace scn;

CycleStateFetchBlockchain::CycleStateFetchBlockchain(BlockchainManager& base)
:base_(base)
,baseline_block_fetch_agent_(nullptr) {

}


CycleStateFetchBlockchain::~CycleStateFetchBlockchain() = default;


void CycleStateFetchBlockchain::onEnter() {
    LOG(INFO) << "enter fetch blockchain state";

    global_blockchain_newest_block_id_ = 0;
    base_.blockchain_.initEmptyChain();
    running_ = true;
    fetch_blocks_thread_ = std::make_shared<std::thread>(&CycleStateFetchBlockchain::fetchBlocksThread, this);
}


bool CycleStateFetchBlockchain::onCycle() {
    {
        LOCK_MUTEX_WATCHDOG_REC(mtx_baseline_block_fetch_agent_);
        if (baseline_block_fetch_agent_) {
            baseline_block_fetch_agent_->onCycle();
        }
    }
    {
        LOCK_MUTEX_WATCHDOG_REC(mtx_block_fetch_agent_map_);
        for(auto& elem : block_fetch_agent_map_) {
            elem.second.onCycle();
        }
    }
    return false;
}


void CycleStateFetchBlockchain::onExit() {
    base_.new_block_ = CollectionBlock();
    base_.new_block_.transactions.clear();
    base_.new_block_.creations.clear();

    base_.out_of_sync_detector_.restartCheckCycle(base_.blockchain_);
    running_ = false;
    fetch_blocks_thread_->join();
}


void CycleStateFetchBlockchain::blockReceivedCallback(const peer_id_t& peer_id, const std::shared_ptr<const BaselineBlock>& block, bool reply) {
    if(reply) {
        LOCK_MUTEX_WATCHDOG_REC(mtx_baseline_block_fetch_agent_);
        if(baseline_block_fetch_agent_) {
            baseline_block_fetch_agent_->blockReceivedCallback(peer_id, block);
        }
    }
}


void CycleStateFetchBlockchain::blockReceivedCallback(const peer_id_t& peer_id, const std::shared_ptr<const CollectionBlock>& block, bool reply) {
    if(reply) {
        LOCK_MUTEX_WATCHDOG_REC(mtx_block_fetch_agent_map_);
        for(auto& elem : block_fetch_agent_map_) {
            if(block->header.block_uid == elem.first) {
                elem.second.blockReceivedCallback(peer_id, block);
                break;
            }
        }
    }
}

bool CycleStateFetchBlockchain::isSynchronized() const {
    return percentSynchronizationDone() == 100;
}

uint8_t CycleStateFetchBlockchain::percentSynchronizationDone() const {
    block_uid_t local_global_blockchain_newest_block_id = global_blockchain_newest_block_id_;
    if(local_global_blockchain_newest_block_id == 0) {
        return 0;
    }
    auto root_block_id = base_.blockchain_.getRootBlockId();
    auto newest_block_id = base_.blockchain_.getNewestBlockId();
    auto current_block_to_ask_for = newest_block_id + 1;
    if(current_block_to_ask_for <= root_block_id ||
            local_global_blockchain_newest_block_id < root_block_id ||
       root_block_id == newest_block_id) {
        return 0;
    }

    uint32_t percent = 100;
    if(local_global_blockchain_newest_block_id - root_block_id + 1 != 0) {
        percent = 100 * static_cast<uint32_t>(current_block_to_ask_for - root_block_id) /
                static_cast<uint32_t>(local_global_blockchain_newest_block_id - root_block_id + 1);
        percent = std::min(percent, 100u);
    }
    return percent;
}

block_uid_t CycleStateFetchBlockchain::fetchBaseline() {
    {
        LOCK_MUTEX_WATCHDOG_REC(mtx_baseline_block_fetch_agent_);
        baseline_block_fetch_agent_ = std::make_shared<BlockFetchAgent<BaselineBlock>>(0, base_.p2p_connector_, base_.sync_timer_, 60000);
    }
    while(running_) {
        {
            LOCK_MUTEX_WATCHDOG_REC(mtx_baseline_block_fetch_agent_);
            auto received_block = baseline_block_fetch_agent_->getReceivedBlock();
            if(received_block != nullptr) {
                if(base_.blockchain_.getNewestBlockId() <= received_block->second->header.block_uid) {
                    if(Blockchain::validateBlockWithoutContext(*received_block->second)) {
                        base_.blockchain_.setRootBlock(*received_block->second);
                        LOG(INFO) << "Fetched block " << received_block->second->header.block_uid << ": "
                                  << hash_helper::toString(received_block->second->header.generic_header.block_hash);
                        block_uid_t ret = received_block->second->header.block_uid + 1;
                        baseline_block_fetch_agent_ = nullptr;
                        return ret;
                    } else {
                        LOG(ERROR) << "Received invalid baseline block";
                        base_.peers_monitor_.reportViolation(received_block->first);
                    }
                }
                else {
                    LOG(INFO) << "Ignoring received baseline block: "
                              << received_block->second->header.block_uid << ": "
                              << hash_helper::toString(received_block->second->header.generic_header.block_hash);
                }
                baseline_block_fetch_agent_->restart();
            }
        }

        global_blockchain_newest_block_id_ = BlockchainManager::getBlockId(base_.sync_timer_.now());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    {
        LOCK_MUTEX_WATCHDOG_REC(mtx_baseline_block_fetch_agent_);
        baseline_block_fetch_agent_ = nullptr;
    }
    return 0;
}

void CycleStateFetchBlockchain::refillAgentMap(block_uid_t& next_id_to_fetch) {
    LOCK_MUTEX_WATCHDOG_REC(mtx_block_fetch_agent_map_);
    while(block_fetch_agent_map_.size() < max_parallel_block_fetchers_ && !BlockchainManager::isBaselineBlock(next_id_to_fetch)) {
        block_fetch_agent_map_.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(next_id_to_fetch),
                                       std::forward_as_tuple(next_id_to_fetch, base_.p2p_connector_, base_.sync_timer_));
        next_id_to_fetch++;
    }
}

void CycleStateFetchBlockchain::processFinishedAgents() {
    LOCK_MUTEX_WATCHDOG_REC(mtx_block_fetch_agent_map_);
    auto it = block_fetch_agent_map_.begin();
    while(it != block_fetch_agent_map_.end()) {
        auto received_block = it->second.getReceivedBlock();
        if(received_block == nullptr) {
            break;
        } else {
            if (base_.blockchain_.validateBlock(*received_block->second)) {
                base_.blockchain_.addBlock(*received_block->second);
                LOG(INFO) << "Fetched block " << received_block->second->header.block_uid << ": "
                          << hash_helper::toString(received_block->second->header.generic_header.block_hash);
            } else {
                LOG(ERROR) << "Received invalid collection block";
                base_.peers_monitor_.reportViolation(received_block->first);
            }
            it = block_fetch_agent_map_.erase(it);
        }
    }
}

void CycleStateFetchBlockchain::fetchBlocksThread() {
    while(running_ && base_.p2p_connector_.numConnectedPeers() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    auto next_id_to_fetch = fetchBaseline();

    while(running_) {
        processFinishedAgents();

        refillAgentMap(next_id_to_fetch);

        global_blockchain_newest_block_id_ = BlockchainManager::getBlockId(base_.sync_timer_.now());

        if(BlockchainManager::getNextBaselineBlock(next_id_to_fetch) <= global_blockchain_newest_block_id_) {
            LOG(INFO) << "Fast forward to new baseline block...";
            next_id_to_fetch = fetchBaseline();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}