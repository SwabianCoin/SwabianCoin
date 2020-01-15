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
#include "scn/P2PConnector/P2PConnector.h"
#include <atomic>

namespace scn {

    class OutOfSyncDetector {
    public:
        OutOfSyncDetector();

        virtual ~OutOfSyncDetector();

        virtual void restartCheckCycle(Blockchain& blockchain);

        virtual bool isOutOfSync() const;

        virtual void blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply);

    protected:
        std::atomic<bool> out_of_sync;
        hash_t newest_block_hash;
        block_uid_t newest_block_id;
        std::mutex mtx_peer_in_sync_map_access_;
        std::map<IPeer*, bool> peer_in_sync_map_;
    };

}


#endif //FULL_NODE_OUTOFSYNCDETECTOR_H
