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

#ifndef FULL_NODE_ICYCLESTATE_H
#define FULL_NODE_ICYCLESTATE_H

#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/P2PConnector/P2PConnector.h"

namespace scn {

    class ICycleState {
    public:

        enum class State : uint8_t {
            Unassigned        = 0,
            FetchBlockchain   = 1,
            Collect           = 2,
            IntroduceBlock    = 3,
            IntroduceBaseline = 4
        };

        ICycleState() = default;

        virtual ~ICycleState() = default;

        virtual void onEnter() = 0;

        virtual bool onCycle() = 0; //return true if there is still something to do

        virtual void onExit() = 0;

        virtual void blockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const BaselineBlock> block, bool reply) {}

        virtual void blockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const CollectionBlock> block, bool reply) {}

        virtual State getState() const = 0;
    };

}

#endif //FULL_NODE_ICYCLESTATE_H
