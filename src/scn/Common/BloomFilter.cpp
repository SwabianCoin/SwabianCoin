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

#include "BloomFilter.h"

//NOTE: This code has been taken and modified from libTorrent (https://www.libtorrent.org/)

using namespace scn;

bool scn::hasBits(const hash_t& hash, uint8_t const* bits, uint32_t const len)
{
    uint32_t idx1 = static_cast<uint32_t>(hash);
    uint32_t idx2 = static_cast<uint32_t>(hash >> 32);
    idx1 %= static_cast<uint32_t>(len * 8);
    idx2 %= static_cast<uint32_t>(len * 8);
    return (bits[idx1 / 8] & (1 << (idx1 & 7))) != 0
           && (bits[idx2 / 8] & (1 << (idx2 & 7))) != 0;
}

void scn::setBits(const hash_t& hash, uint8_t* bits, uint32_t const len)
{
    uint32_t idx1 = static_cast<uint32_t>(hash);
    uint32_t idx2 = static_cast<uint32_t>(hash >> 32);
    idx1 %= static_cast<uint32_t>(len * 8);
    idx2 %= static_cast<uint32_t>(len * 8);
    bits[idx1 / 8] |= (1 << (idx1 & 7));
    bits[idx2 / 8] |= (1 << (idx2 & 7));
}

uint32_t scn::countZeroBits(uint8_t const* bits, uint32_t const len)
{
    // number of bits _not_ set in a nibble
    uint8_t bitcount[16] =
            {
                    // 0000, 0001, 0010, 0011, 0100, 0101, 0110, 0111,
                    // 1000, 1001, 1010, 1011, 1100, 1101, 1110, 1111
                    4, 3, 3, 2, 3, 2, 2, 1,
                    3, 2, 2, 1, 2, 1, 1, 0
            };
    uint32_t ret = 0;
    for (uint32_t i = 0; i < len; ++i)
    {
        ret += bitcount[bits[i] & 0xf];
        ret += bitcount[(bits[i] >> 4) & 0xf];
    }
    return ret;
}