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

#ifndef FULL_NODE_ENTRYPOINTFETCHER_H
#define FULL_NODE_ENTRYPOINTFETCHER_H

#include "scn/Common/Common.h"
#include <list>

namespace scn {

    class EntryPointFetcher {
    public:
        EntryPointFetcher() {};
        virtual ~EntryPointFetcher() {};

        static const std::list<std::pair<std::string, std::uint16_t>> fetch();

    protected:
        static const std::string entry_points_url_;
        static const std::list<std::pair<std::string, std::uint16_t>> entry_points_fallback_;
    };

}

#endif //FULL_NODE_ENTRYPOINTFETCHER_H
