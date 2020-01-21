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


#ifndef FULL_NODE_BLOOMFILTER_H
#define FULL_NODE_BLOOMFILTER_H

#include "scn/Common/Common.h"
#include <cereal/types/array.hpp>

//NOTE: This code has been taken and modified from libTorrent (https://www.libtorrent.org/)

namespace scn {

    //NOTE: The bloom filter parameter K is fixed to 2 in this implementation

    void setBits(const hash_t& hash, uint8_t* bits, uint32_t len);
    bool hasBits(const hash_t& hash, uint8_t const* bits, uint32_t len);
    uint32_t countZeroBits(uint8_t const* bits, uint32_t len);

    template<uint32_t M_BYTE> class BloomFilter {
    public:
        BloomFilter(){
            clear();
        }

        bool findHash(const hash_t& hash) {
            return hasBits(hash, bloom_filter_.data(), M_BYTE);
        }

        void insertHash(const hash_t& hash) {
            setBits(hash, bloom_filter_.data(), M_BYTE);
        }

        void merge(const BloomFilter<M_BYTE>& filter_to_merge) {
            for(uint32_t i=0;i<M_BYTE;i++) {
                bloom_filter_[i] |= filter_to_merge.bloom_filter_[i];
            }
        }

        void clear() {
            std::memset(bloom_filter_.data(), 0, M_BYTE);
        }

        double numHashEstimation() const{
            const uint32_t c = std::min(countZeroBits(bloom_filter_.data(), M_BYTE), (M_BYTE * 8) - 1);
            const int m = M_BYTE * 8;
            return std::log(c / float(m)) / (2.f * std::log(1.f - 1.f/m));
        }

        template<class Archive>
        void ser(Archive& ar) {
            ar & bloom_filter_;
        }

    protected:

        std::array<uint8_t, M_BYTE> bloom_filter_;
    };

}


#endif //FULL_NODE_BLOOMFILTER_H
