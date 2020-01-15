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
        ICycleState() {};

        virtual ~ICycleState() {};

        virtual void onEnter() = 0;

        virtual bool onCycle() = 0; //return true if there is still something to do

        virtual void onExit() = 0;

        virtual void blockReceivedCallback(IPeer& peer, const BaselineBlock &block, bool reply) {}

        virtual void blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) {}
    };

}

#endif //FULL_NODE_ICYCLESTATE_H
