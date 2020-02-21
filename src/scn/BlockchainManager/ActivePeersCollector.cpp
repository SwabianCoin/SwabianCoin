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


#include "ActivePeersCollector.h"
#include "scn/CryptoHelper/CryptoHelper.h"
#include "libtorrent/bloom_filter.hpp"

using namespace scn;


ActivePeersCollector::ActivePeersCollector(IP2PConnector& p2p_connector, public_key_t& owner_key)
:p2p_connector_(p2p_connector)
,owner_key_hash_(CryptoHelper::calcHash(owner_key.getAsShortString()))
,temp_active_peers_list_()
,total_active_peers(0) {
    p2p_connector_.registerActivePeersCallback(std::bind(&ActivePeersCollector::activePeersListReceivedCallback, this, std::placeholders::_1, std::placeholders::_2));
}


ActivePeersCollector::~ActivePeersCollector() = default;


void ActivePeersCollector::activePeersListReceivedCallback(const peer_id_t& peer_id, const ActivePeersList& active_peers_list) {
    LOCK_MUTEX_WATCHDOG(mtx_temp_active_peers_list_);
    temp_active_peers_list_.active_peers_bloom_filter.merge(active_peers_list.active_peers_bloom_filter);
}


void ActivePeersCollector::restartListBuilding() {
    LOCK_MUTEX_WATCHDOG(mtx_temp_active_peers_list_);
    total_active_peers = temp_active_peers_list_.active_peers_bloom_filter.numHashEstimation();
    temp_active_peers_list_.active_peers_bloom_filter.clear();
    temp_active_peers_list_.active_peers_bloom_filter.insertHash(owner_key_hash_);
}


void ActivePeersCollector::propagate() {
    ActivePeersList local_active_peers_list;
    {
        LOCK_MUTEX_WATCHDOG(mtx_temp_active_peers_list_);
        local_active_peers_list = temp_active_peers_list_;
    }
    p2p_connector_.propagateActivePeersList(local_active_peers_list);
}


uint64_t ActivePeersCollector::getActivePeers() const {
    LOCK_MUTEX_WATCHDOG(mtx_temp_active_peers_list_);
    return std::max(total_active_peers, static_cast<uint64_t>(temp_active_peers_list_.active_peers_bloom_filter.numHashEstimation()));
}