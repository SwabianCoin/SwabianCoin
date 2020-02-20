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

#ifndef FULL_NODE_BLOCKFETCHAGENT_H
#define FULL_NODE_BLOCKFETCHAGENT_H

#include "scn/Common/Common.h"
#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/P2PConnector/IP2PConnector.h"
#include "scn/SynchronizedTime/ISynchronizedTimer.h"
#include <mutex>


namespace scn {

    template<class BLOCK_TYPE>
    class BlockFetchAgent {
    public:
        explicit BlockFetchAgent(const block_uid_t block_id, IP2PConnector& p2p_connector, ISynchronizedTimer& sync_timer, const uint32_t fetch_timeout_ms = 2000);

        virtual void onCycle();

        virtual std::shared_ptr<std::pair<const peer_id_t, std::shared_ptr<const BLOCK_TYPE>>> getReceivedBlock();

        virtual void blockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const BLOCK_TYPE> block);

        void restart();

    protected:
        const uint32_t fetch_timeout_ms_;

        block_uid_t block_id_;
        IP2PConnector& p2p_connector_;
        ISynchronizedTimer& sync_timer_;

        std::mutex mtx_received_block_access_;
        std::shared_ptr<std::pair<const peer_id_t, std::shared_ptr<const BLOCK_TYPE>>> received_block_;

        blockchain_time_t next_fetch_time_;
    };

    template<class BLOCK_TYPE>
    BlockFetchAgent<BLOCK_TYPE>::BlockFetchAgent(const block_uid_t block_id, IP2PConnector& p2p_connector, ISynchronizedTimer& sync_timer, const uint32_t fetch_timeout_ms)
            :fetch_timeout_ms_(fetch_timeout_ms)
            ,block_id_(block_id)
            ,p2p_connector_(p2p_connector)
            ,sync_timer_(sync_timer)
            ,received_block_(nullptr) {
        next_fetch_time_ = sync_timer_.now();
    }

    template<class BLOCK_TYPE>
    std::shared_ptr<std::pair<const peer_id_t, std::shared_ptr<const BLOCK_TYPE>>> BlockFetchAgent<BLOCK_TYPE>::getReceivedBlock() {
        LOCK_MUTEX_WATCHDOG(mtx_received_block_access_);
        return received_block_;
    }

    template<class BLOCK_TYPE>
    void BlockFetchAgent<BLOCK_TYPE>::blockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const BLOCK_TYPE> block) {
        LOCK_MUTEX_WATCHDOG(mtx_received_block_access_);
        if(received_block_ == nullptr && (block_id_ == 0 || block->header.block_uid == block_id_)) {
            received_block_ = std::make_shared<std::pair<const peer_id_t, std::shared_ptr<const BLOCK_TYPE>>>(peer_id, block);
        }
    }

    template<class BLOCK_TYPE>
    void BlockFetchAgent<BLOCK_TYPE>::restart() {
        LOCK_MUTEX_WATCHDOG(mtx_received_block_access_);
        received_block_ = nullptr;
        next_fetch_time_ = sync_timer_.now();
    }

}

#endif //FULL_NODE_BLOCKFETCHAGENT_H
