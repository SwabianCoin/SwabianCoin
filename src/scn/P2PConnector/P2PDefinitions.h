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

#ifndef FULL_NODE_P2PDEFINITIONS_H
#define FULL_NODE_P2PDEFINITIONS_H

#include <cstdint>
#include <set>
#include "scn/Common/Common.h"
#include "scn/Common/BloomFilter.h"

namespace scn {

    enum class MessageType : uint8_t {
        PropagateBaselineBlock = 1,
        PropagateCollectionBlock = 2,
        AskForBlock = 5,
        AskForLastBaselineBlock = 6,
        PropagateActivePeersList = 7
    };

    struct ActivePeersList {
        BloomFilter<8192> active_peers_bloom_filter;

        ActivePeersList()
        : active_peers_bloom_filter() {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & active_peers_bloom_filter;
        }
    };

}

#endif //FULL_NODE_P2PDEFINITIONS_H
