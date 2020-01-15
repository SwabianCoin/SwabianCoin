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

#ifndef FULL_NODE_CYCLESTATEINTRODUCEBASELINE_H
#define FULL_NODE_CYCLESTATEINTRODUCEBASELINE_H

#include "ICycleState.h"


namespace scn {

    class BlockchainManager;

    class CycleStateIntroduceBaseline : public ICycleState {
    public:
        CycleStateIntroduceBaseline(BlockchainManager& base);

        virtual ~CycleStateIntroduceBaseline();

        virtual void onEnter() override;

        virtual bool onCycle() override;

        virtual void onExit() override;

    protected:
        BlockchainManager& base_;
    };

}

#endif //FULL_NODE_CYCLESTATEINTRODUCEBASELINE_H
