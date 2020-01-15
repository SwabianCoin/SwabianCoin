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


PeersMonitor::PeersMonitor(ISynchronizedTimer& sync_timer)
: sync_timer_(sync_timer) {

}


PeersMonitor::~PeersMonitor() {

}


void PeersMonitor::blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) {
    if(!reply) {
        //update list
        auto &history_list = peer_message_history_[peer.getId()];
        history_list.push_back(sync_timer_.now());
        if (history_list.size() > message_history_size_) {
            history_list.pop_front();
        }

        //update violations
        if ((history_list.size() == message_history_size_) && (history_list.back() - history_list.front() <
                                                               (min_allowed_avg_time_between_propagations_ms_ *
                                                                (message_history_size_ - 1)))) {
            history_list.clear();
            LOG(INFO) << "Peer is sending messages faster than allowed: " << history_list.front() << " " << history_list.back() << " " << history_list.size();
            reportViolation(peer);
        }
    }
}


void PeersMonitor::reportViolation(IPeer& peer) {
    bool ban = false;
    {
        LOCK_MUTEX_WATCHDOG(mtx_peer_violations_map_access_);
        //update violations
        auto &violations = peer_violations_map_[peer.getId()];
        violations++;
        LOG(INFO) << "Peer violation - peer: " << peer.getId() << " violations: " << violations;

        //kick if necessary
        if (violations > num_tolerated_violations_) {
            ban = true;
        }
    }
    if(ban) {
        LOG(WARNING) << "Ban peer (too much violations): " << peer.getInfo() << " - " << peer.getId();
        peer.ban();
    }
}
