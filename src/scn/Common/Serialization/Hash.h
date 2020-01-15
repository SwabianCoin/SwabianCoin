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

#ifndef FULL_NODE_HASH_H
#define FULL_NODE_HASH_H

#include "scn/Common/Common.h"

namespace cereal {

    //serialization of boost multiprecision type
    template<class Archive>
    void save(Archive &archive,
              scn::hash_t const &m) {
        for (auto i = 3; i >= 0; i--) {
            archive << (uint64_t) (m >> (i * 64));
        }
    }

    template<class Archive>
    void load(Archive &archive,
              scn::hash_t &m) {
        m = 0;
        for (auto i = 0; i < 4; i++) {
            m = (m << 64);
            uint64_t buffer;
            archive >> buffer;
            m += buffer;
        }
    }

}

#endif //FULL_NODE_HASH_H
