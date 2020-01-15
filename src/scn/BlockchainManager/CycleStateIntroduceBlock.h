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

#ifndef FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H
#define FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H

#include "ICycleState.h"


namespace scn {

    class BlockchainManager;

    class CycleStateIntroduceBlock : public ICycleState {
    public:
        CycleStateIntroduceBlock(BlockchainManager& base);

        virtual ~CycleStateIntroduceBlock();

        virtual void onEnter() override;

        virtual bool onCycle() override;

        virtual void onExit() override;

        virtual void blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) override;

        static const blockchain_time_t time_between_propagations_ms_ = 4000;

    protected:

        BlockchainManager& base_;
        blockchain_time_t next_propagation_time_;

        std::set<hash_t> processed_blocks_;
    };

}

#endif //FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H
