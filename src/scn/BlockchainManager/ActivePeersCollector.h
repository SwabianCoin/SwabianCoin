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


#ifndef FULL_NODE_ACTIVEPEERSCOLLECTOR_H
#define FULL_NODE_ACTIVEPEERSCOLLECTOR_H

#include "scn/Common/Common.h"
#include "scn/P2PConnector/P2PDefinitions.h"
#include "scn/P2PConnector/IP2PConnector.h"
#include <mutex>

namespace scn {

    class ActivePeersCollector {
    public:

        ActivePeersCollector(IP2PConnector& p2p_connector, public_key_t& owner_key);
        virtual ~ActivePeersCollector();

        virtual void activePeersListReceivedCallback(const peer_id_t& peer_id, const ActivePeersList& active_peers_list);

        virtual void restartListBuilding();

        virtual void propagate();

        virtual uint64_t getActivePeers() const;

        static const uint32_t blocks_to_collect_active_peers_ = 30;

    protected:

        IP2PConnector& p2p_connector_;
        hash_t owner_key_hash_;
        mutable std::mutex mtx_temp_active_peers_list_;
        ActivePeersList temp_active_peers_list_;
        uint64_t total_active_peers;
    };

}

#endif //FULL_NODE_ACTIVEPEERSCOLLECTOR_H
