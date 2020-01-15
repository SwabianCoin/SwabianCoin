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

#include "OutOfSyncDetector.h"

using namespace scn;


OutOfSyncDetector::OutOfSyncDetector()
: out_of_sync(false)
, newest_block_hash(0)
, newest_block_id(0) {

}


OutOfSyncDetector::~OutOfSyncDetector() {

}


void OutOfSyncDetector::restartCheckCycle(Blockchain& blockchain) {
    LOCK_MUTEX_WATCHDOG(mtx_peer_in_sync_map_access_);
    out_of_sync = false;
    auto newest_block = blockchain.getNewestBlock();
    newest_block_hash = newest_block->header.generic_header.block_hash;
    newest_block_id = newest_block->header.block_uid;
    peer_in_sync_map_.clear();
}


bool OutOfSyncDetector::isOutOfSync() const {
    return out_of_sync;
}


void OutOfSyncDetector::blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply) {
    if(!reply && block.header.block_uid == newest_block_id+1) {
        LOCK_MUTEX_WATCHDOG(mtx_peer_in_sync_map_access_);
        bool hash_in_sync_with_us = (block.header.generic_header.previous_block_hash == newest_block_hash);
        peer_in_sync_map_[&peer] = hash_in_sync_with_us;
        if(!hash_in_sync_with_us) {
            LOG(INFO) << "Peer not in sync: " << peer.getInfo() << " - our previous hash: " << newest_block_hash << " - theirs: " << block.header.generic_header.previous_block_hash;
        }
        uint32_t peers_out_of_sync = 0;
        uint32_t num_peers = peer_in_sync_map_.size();
        for(auto& elem : peer_in_sync_map_) {
            if(!elem.second) {
                peers_out_of_sync++;
            }
        }
        out_of_sync = (static_cast<double>(peers_out_of_sync) / static_cast<double>(num_peers)) > 0.5;
        if(out_of_sync) {
            LOG(INFO) << "Out of sync detected - num_peers: " << num_peers << " num_peers_out_of_sync: " << peers_out_of_sync;
        }
    }
}