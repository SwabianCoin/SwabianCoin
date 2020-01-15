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
,synchronized_(false)
,next_block_to_ask_for_(0) {

}


CycleStateFetchBlockchain::~CycleStateFetchBlockchain() {

}


void CycleStateFetchBlockchain::onEnter() {
    LOG(INFO) << "enter fetch blockchain state";

    synchronized_ = false;
    global_blockchain_newest_block_id_ = 0;
    {
        LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
        next_block_to_ask_for_ = 0;
    }
    base_.blockchain_.initEmptyChain();
    running_ = true;
    fetch_blocks_thread_ = std::make_shared<std::thread>(&CycleStateFetchBlockchain::fetchBlocksThread, this);
}


bool CycleStateFetchBlockchain::onCycle() {
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


void CycleStateFetchBlockchain::blockReceivedCallback(IPeer& peer, const BaselineBlock &block, bool reply) {
    if(reply) {
        if(base_.blockchain_.getNewestBlockId() <= block.header.block_uid) {
            if(Blockchain::validateBlockWithoutContext(block)) {
                base_.blockchain_.setRootBlock(block);
                LOG(INFO) << "Fetched block " << block.header.block_uid << ": "
                          << block.header.generic_header.block_hash.str(0, std::ios_base::hex);
                {
                    LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
                    next_block_to_ask_for_ = block.header.block_uid + 1;
                }
            } else {
                LOG(ERROR) << "Received invalid baseline block";
                base_.peers_monitor_.reportViolation(peer);
            }
        }
        else {
            LOG(INFO) << "Ignoring received baseline block: "
                      << block.header.block_uid << ": "
                      << block.header.generic_header.block_hash.str(0, std::ios_base::hex);
        }
    } else {
        synchronized_ = (block.header.block_uid == base_.blockchain_.getNewestBlockId()+1);
    }
}


void CycleStateFetchBlockchain::blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) {
    if(reply) {
        if(block.header.block_uid == base_.blockchain_.getNewestBlockId()+1) {
            if (base_.blockchain_.validateBlock(block)) {
                base_.blockchain_.addBlock(block);
                LOG(INFO) << "Fetched block " << block.header.block_uid << ": "
                          << block.header.generic_header.block_hash.str(0, std::ios_base::hex);
                {
                    LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
                    next_block_to_ask_for_ = block.header.block_uid + 1;
                }
            } else {
                LOG(ERROR) << "Received invalid collection block";
                base_.peers_monitor_.reportViolation(peer);
            }
        }
        else {
            LOG(INFO) << "Received block is not the one we need: " << block.header.block_uid;
        }
    } else {
        synchronized_ = (block.header.block_uid == base_.blockchain_.getNewestBlockId()+1);
        global_blockchain_newest_block_id_ = block.header.block_uid - 1;
    }
}

bool CycleStateFetchBlockchain::isSynchronized() const {
    return synchronized_;
}

uint8_t CycleStateFetchBlockchain::percentSynchronizationDone() const {
    if(global_blockchain_newest_block_id_ == 0) {
        return 0;
    }
    auto root_block_id = base_.blockchain_.getRootBlockId();
    block_uid_t current_block_to_ask_for = 0;
    {
        std::lock_guard<std::mutex> lock(mtx_next_block_to_ask_for_);
        current_block_to_ask_for = next_block_to_ask_for_;
    }
    if(current_block_to_ask_for <= root_block_id || global_blockchain_newest_block_id_ < root_block_id) {
        return 0;
    }

    uint32_t percent = 100;
    if(global_blockchain_newest_block_id_ - root_block_id != 0) {
        percent = 100 * static_cast<uint32_t>(current_block_to_ask_for - 1 - root_block_id) /
                static_cast<uint32_t>(global_blockchain_newest_block_id_ - root_block_id);
    }
    return percent;
}

void CycleStateFetchBlockchain::fetchBlocksThread() {
    while(running_ && base_.p2p_connector_.numConnectedPeers() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while(running_) {
        block_uid_t current_block_to_ask_for;
        {
            LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
            current_block_to_ask_for = next_block_to_ask_for_;
        }

        if(current_block_to_ask_for == 0) {
            base_.p2p_connector_.askForLastBaselineBlock();
            LOG(INFO) << "Asking for baseline block...";
        } else  {
            if(base_.getNextBaselineBlock(current_block_to_ask_for) <= global_blockchain_newest_block_id_) {
                base_.p2p_connector_.askForLastBaselineBlock();
                LOG(INFO) << "Fast forward to new baseline block...";
                {
                    LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
                    next_block_to_ask_for_ = 0;
                    current_block_to_ask_for = next_block_to_ask_for_;
                }
            } else {
                base_.p2p_connector_.askForBlock(current_block_to_ask_for);
                LOG(INFO) << "Asking for block " << current_block_to_ask_for << "...";
            }
        }

        uint32_t wait_time_s = 10;
        if(current_block_to_ask_for == 0 || BlockchainManager::isBaselineBlock(current_block_to_ask_for)) {
            wait_time_s = 60;
        }

        for(uint32_t i=0;i<wait_time_s*10;i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if(!running_) {
                break;
            }

            {
                LOCK_MUTEX_WATCHDOG(mtx_next_block_to_ask_for_);
                if(next_block_to_ask_for_ != current_block_to_ask_for)  {
                    break;
                }
            }
        }

    }
}