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

#ifndef FULL_NODE_OUTOFSYNCDETECTOR_H
#define FULL_NODE_OUTOFSYNCDETECTOR_H

#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/Blockchain/Blockchain.h"
#include "scn/SynchronizedTime/ISynchronizedTimer.h"
#include <atomic>

namespace scn {

    class OutOfSyncDetector {
    public:
        explicit OutOfSyncDetector(ISynchronizedTimer& sync_timer);

        virtual ~OutOfSyncDetector();

        virtual void restartCheckCycle(Blockchain& blockchain);

        virtual bool isOutOfSync() const;

        virtual void blockReceivedCallback(const peer_id_t& peer_id, const CollectionBlock &block, bool reply);

    protected:
        ISynchronizedTimer& sync_timer_;

        std::atomic<bool> input_msgs_out_of_sync_;
        hash_t newest_block_hash_;
        block_uid_t newest_block_id_;
        std::mutex mtx_peer_in_sync_map_access_;
        std::map<peer_id_t, bool> peer_in_sync_map_;

        std::atomic<bool> time_out_of_sync_;
    };

}


#endif //FULL_NODE_OUTOFSYNCDETECTOR_H
