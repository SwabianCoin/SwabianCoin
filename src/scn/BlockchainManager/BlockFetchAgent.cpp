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

#include "BlockFetchAgent.h"


namespace scn {

    template<>
    void BlockFetchAgent<CollectionBlock>::onCycle() {
        if (sync_timer_.now() >= next_fetch_time_) {
            p2p_connector_.askForBlock(block_id_);
            next_fetch_time_ += fetch_timeout_ms_;
        }
    }

    template<>
    void BlockFetchAgent<BaselineBlock>::onCycle() {
        if (sync_timer_.now() >= next_fetch_time_) {
            p2p_connector_.askForLastBaselineBlock();
            next_fetch_time_ += fetch_timeout_ms_;
        }
    }

}