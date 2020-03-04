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

#ifndef FULL_NODE_ENTRYPOINTFETCHERSTUB_H
#define FULL_NODE_ENTRYPOINTFETCHERSTUB_H

#include "scn/P2PConnector/IEntryPointFetcher.h"

namespace scn {

    class EntryPointFetcherStub : public IEntryPointFetcher {
    public:
        EntryPointFetcherStub()
        :num_calls_fetch_(0)
        ,entry_point_list_() {}

        virtual ~EntryPointFetcherStub() = default;

        const std::list<std::pair<std::string, std::uint16_t>> fetch() override {
            num_calls_fetch_++;
            return entry_point_list_;
        }

        uint32_t num_calls_fetch_;
        std::list<std::pair<std::string, std::uint16_t>> entry_point_list_;
    };

}

#endif //FULL_NODE_ENTRYPOINTFETCHERSTUB_H
