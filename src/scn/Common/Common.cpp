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

#include "Common.h"

using namespace scn;

std::string scn::hash_helper::toString(const hash_t& hash) {
    return hash.str(0, std::ios_base::hex | std::ios_base::uppercase);
}

void scn::hash_helper::toArray(const hash_t& hash, uint8_t* array) {
    hash_t temp = hash;
    for(auto i=0;i<32;i++) {
        array[32-1-i] = (uint8_t)temp;
        temp >>= 8;
    }
}

hash_t scn::hash_helper::fromArray(const uint8_t* array) {
    hash_t ret = 0;
    for(auto i=0;i<32;i++) {
        ret <<=  8;
        ret |= array[i];
    }
    return ret;
}