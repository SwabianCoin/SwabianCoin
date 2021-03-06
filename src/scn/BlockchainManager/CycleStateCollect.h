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

#ifndef FULL_NODE_CYCLESTATECOLLECT_H
#define FULL_NODE_CYCLESTATECOLLECT_H

#include "ICycleState.h"


namespace scn {

    class BlockchainManager;

    class CycleStateCollect : public ICycleState {
    public:
        explicit CycleStateCollect(BlockchainManager& base);

        ~CycleStateCollect() override;

        void onEnter() override;

        bool onCycle() override;

        void onExit() override;

        State getState() const override { return State::Collect; }

    protected:
        BlockchainManager& base_;
        uint32_t minings_in_cycle_;
        uint32_t max_cycle_duration_ms_;
    };

}

#endif //FULL_NODE_CYCLESTATECOLLECT_H
