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

#ifndef FULL_NODE_IP2PCONNECTOR_H
#define FULL_NODE_IP2PCONNECTOR_H

#include "scn/Common/Common.h"
#include "scn/Blockchain/BlockDefinitions.h"
#include "P2PDefinitions.h"
#include "libtorrent/extensions/IPeer.h"

namespace scn {

    typedef libtorrent::IPeer IPeer; //todo: find better solution than typedef to libtorrent type here

    class IP2PConnector {
    public:
        IP2PConnector() {};
        virtual ~IP2PConnector() {};

        virtual void connect() = 0;

        virtual void disconnect() = 0;

        virtual uint32_t numConnectedPeers() const = 0;

        virtual void registerBlockCallbacks(std::function<void(IPeer&, const BaselineBlock&,bool)> callback_baseline,
                                            std::function<void(IPeer&, const CollectionBlock&,bool)> callback_collection) = 0;

        virtual void askForBlock(block_uid_t uid) = 0;

        virtual void askForLastBaselineBlock() = 0;

        virtual void propagateBlock(const BaselineBlock& block) = 0;

        virtual void propagateBlock(const CollectionBlock& block) = 0;
    };

}

#endif //FULL_NODE_IP2PCONNECTOR_H
