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
#include <mutex>
#include <thread>


namespace scn {

    class BlockchainManager;

    class CycleStateFetchBlockchain : public ICycleState {
    public:
        CycleStateFetchBlockchain(BlockchainManager& base);

        virtual ~CycleStateFetchBlockchain();

        virtual void onEnter() override;

        virtual bool onCycle() override;

        virtual void onExit() override;

        virtual void blockReceivedCallback(IPeer& peer, const BaselineBlock &block, bool reply) override;

        virtual void blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) override;

        virtual State getState() const { return State::FetchBlockchain; }

        virtual bool isSynchronized() const;

        virtual uint8_t percentSynchronizationDone() const;

    protected:

        virtual void fetchBlocksThread();

        BlockchainManager& base_;

        bool synchronized_;
        block_uid_t global_blockchain_newest_block_id_;
        mutable std::mutex mtx_next_block_to_ask_for_;
        block_uid_t next_block_to_ask_for_;

        bool running_;
        std::shared_ptr<std::thread> fetch_blocks_thread_;
    };

}

#endif //FULL_NODE_CYCLESTATEFETCHBLOCKCHAIN_H
