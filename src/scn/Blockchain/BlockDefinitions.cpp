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

#include "BlockDefinitions.h"

using namespace scn;


std::ostream& scn::operator<<(std::ostream& os, const BaseBlock& block) {
    os << "Block " << block.header.block_uid << std::endl;
    os << "   hash: " << block.header.generic_header.block_hash.str(0, std::ios_base::hex) << std::endl;
    os << "   previous_block_hash: " << block.header.generic_header.previous_block_hash.str(0, std::ios_base::hex)
       << std::endl;
    os << "   type: " << (uint32_t)block.header.generic_header.block_type << std::endl;
    return os;
}


std::ostream& scn::operator<<(std::ostream& os, const BaselineBlock& block) {
    os << (BaseBlock&)block;
    os << "   wallets" << std::endl;
    for(auto& wallet : block.wallets) {
        os << "      " << wallet.first.getAsShortString() << ": " << wallet.second << std::endl;
    }
    os << "   data_value_hashes" << std::endl;
    for(uint32_t epoch = 0;epoch < block.data_value_hashes.size();epoch++) {
        os << "    epoch " << epoch << ": " << block.data_value_hashes[epoch].size() << std::endl;
        for(auto& data_value_hash : block.data_value_hashes[epoch]) {
            os << "     " << data_value_hash.str(0, std::ios_base::hex) << std::endl;
        }

    }
    return os;
}


std::ostream& scn::operator<<(std::ostream& os, const CollectionBlock& block) {
    os << (BaseBlock&)block;
    os << "   transactions" << std::endl;
    for(auto& trans : block.transactions) {
        os << trans.second << std::endl;
    }
    os << "   creations" << std::endl;
    for(auto& creation : block.creations) {
        os << creation.second << std::endl;
    }

    return os;
}


std::ostream& scn::operator<<(std::ostream& os, const TransactionSubBlock& block) {
    os << "TransactionSubBlock " << std::endl;
    os << "   hash: " << block.header.generic_header.block_hash.str(0, std::ios_base::hex) << std::endl;
    os << "   previous_block_hash: " << block.header.generic_header.previous_block_hash.str(0, std::ios_base::hex)
       << std::endl;
    os << "   type: " << (uint32_t)block.header.generic_header.block_type << std::endl;
    os << "   fraction: " << block.fraction << std::endl;
    //os << "   preowner: " << block.pre_owner << std::endl;
    //os << "   postowner: " << block.post_owner << std::endl;
    //os << "   signature: " << block.signature << std::endl;
    return os;
}


std::ostream& scn::operator<<(std::ostream& os, const CreationSubBlock& block) {
    os << "CreationSubBlock " << std::endl;
    os << "   hash: " << block.header.generic_header.block_hash.str(0, std::ios_base::hex) << std::endl;
    os << "   previous_block_hash: " << block.header.generic_header.previous_block_hash.str(0, std::ios_base::hex)
       << std::endl;
    os << "   type: " << (uint32_t)block.header.generic_header.block_type << std::endl;
    //os << "   data value: " << block.data_value << std::endl;
    //os << "   creator: " << block.creator << std::endl;
    //os << "   signature: " << block.signature << std::endl;
    return os;
}