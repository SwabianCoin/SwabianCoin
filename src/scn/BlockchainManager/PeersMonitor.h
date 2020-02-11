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

#ifndef FULL_NODE_PEERSMONITOR_H
#define FULL_NODE_PEERSMONITOR_H

#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/P2PConnector/P2PConnector.h"
#include "scn/SynchronizedTime/ISynchronizedTimer.h"
#include "scn/BlockchainManager/CycleStateIntroduceBlock.h"
#include <mutex>

//todo: watch out for all types of messages (not only CollectionBlocks) to detect if someone is flooding this peer

namespace scn {

    class PeersMonitor {
    public:
        explicit PeersMonitor(ISynchronizedTimer& sync_timer);

        virtual ~PeersMonitor();

        virtual void blockReceivedCallback(IPeer& peer, const CollectionBlock &block, bool reply);

        virtual void reportViolation(IPeer& peer);

    protected:

        static const uint32_t message_history_size_ = 50;
        static const uint32_t num_tolerated_violations_ = 3;
        static const uint32_t min_allowed_avg_time_between_propagations_ms_ = CycleStateIntroduceBlock::time_between_propagations_ms_*4/5;

        ISynchronizedTimer& sync_timer_;
        std::map<std::string, std::list<blockchain_time_t>> peer_message_history_;
        std::mutex mtx_peer_violations_map_access_;
        std::map<std::string, uint32_t> peer_violations_map_;
    };

}

#endif //FULL_NODE_PEERSMONITOR_H
