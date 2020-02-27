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

#include "PeersMonitor.h"

using namespace scn;


PeersMonitor::PeersMonitor(ISynchronizedTimer& sync_timer, IP2PConnector& p2p_connector)
: sync_timer_(sync_timer)
, p2p_connector_(p2p_connector) {

}


PeersMonitor::~PeersMonitor() = default;


void PeersMonitor::blockReceivedCallback(const peer_id_t& peer_id, const CollectionBlock &block, bool reply) {
    if(!reply) {
        //update list
        auto &history_list = peer_message_history_[peer_id];
        history_list.push_back(sync_timer_.now());
        if (history_list.size() > message_history_size_) {
            history_list.pop_front();
        }

        //update violations
        if ((history_list.size() == message_history_size_) && (history_list.back() - history_list.front() <
                                                               (min_allowed_avg_time_between_propagations_ms_ *
                                                                (message_history_size_ - 1)))) {
            LOG(INFO) << "Peer is sending messages faster than allowed: " << history_list.front() << " " << history_list.back() << " " << history_list.size();
            history_list.clear();
            reportViolation(peer_id);
        }
    }
}


void PeersMonitor::reportViolation(const peer_id_t& peer_id) {
    bool ban = false;
    {
        LOCK_MUTEX_WATCHDOG(mtx_peer_violations_map_access_);
        //update violations
        auto &violations = peer_violations_map_[peer_id];
        violations++;
        LOG(INFO) << "Peer violation - peer: " << peer_id << " violations: " << violations;

        //kick if necessary
        if (violations > num_tolerated_violations_) {
            ban = true;
        }
    }
    if(ban) {
        LOG(WARNING) << "Ban peer (too much violations): " << peer_id;
        p2p_connector_.banPeer(peer_id);
    }
}
