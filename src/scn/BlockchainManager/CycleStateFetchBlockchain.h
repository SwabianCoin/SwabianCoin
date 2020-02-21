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

#ifndef FULL_NODE_CYCLESTATEFETCHBLOCKCHAIN_H
#define FULL_NODE_CYCLESTATEFETCHBLOCKCHAIN_H

#include "ICycleState.h"
#include "BlockFetchAgent.h"
#include <mutex>
#include <map>
#include <thread>
#include <atomic>


namespace scn {

    class BlockchainManager;

    class CycleStateFetchBlockchain : public ICycleState {
    public:
        explicit CycleStateFetchBlockchain(BlockchainManager& base);

        ~CycleStateFetchBlockchain() override;

        void onEnter() override;

        bool onCycle() override;

        void onExit() override;

        void blockReceivedCallback(const peer_id_t& peer_id, const std::shared_ptr<const BaselineBlock>& block, bool reply) override;

        void blockReceivedCallback(const peer_id_t& peer_id, const std::shared_ptr<const CollectionBlock>& block, bool reply) override;

        State getState() const override { return State::FetchBlockchain; }

        virtual bool isSynchronized() const;

        virtual uint8_t percentSynchronizationDone() const;

    protected:

        virtual void fetchBlocksThread();

        virtual block_uid_t fetchBaseline();

        virtual void refillAgentMap(block_uid_t& next_id_to_fetch);

        virtual void processFinishedAgents();

        BlockchainManager& base_;

        std::atomic<block_uid_t> global_blockchain_newest_block_id_;

        std::recursive_mutex mtx_baseline_block_fetch_agent_;
        std::shared_ptr<BlockFetchAgent<BaselineBlock>> baseline_block_fetch_agent_;
        std::recursive_mutex mtx_block_fetch_agent_map_;
        std::map<block_uid_t, BlockFetchAgent<CollectionBlock>> block_fetch_agent_map_;
        static const uint32_t max_parallel_block_fetchers_ = 50;

        bool running_;
        std::shared_ptr<std::thread> fetch_blocks_thread_;
    };

}

#endif //FULL_NODE_CYCLESTATEFETCHBLOCKCHAIN_H
