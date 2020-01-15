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

#ifndef FULL_NODE_BLOCKDEFINITIONS_H
#define FULL_NODE_BLOCKDEFINITIONS_H

#include "scn/Common/Common.h"
#include "scn/Common/Serialization/Hash.h"
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/common.hpp>
#include <cereal/cereal.hpp>
#include <iostream>


namespace scn {

    enum class BlockType : uint8_t {
        CollectionBlock = 1,
        BaselineBlock = 2,
        TransactionSubBlock = 3,
        CreationSubBlock = 4
    };

    struct GenericHeader
    {
        hash_t block_hash;
        hash_t previous_block_hash;
        BlockType block_type;

        GenericHeader()
        : block_hash(0)
        , previous_block_hash(0)
        , block_type()
        {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & block_hash;
            ar & previous_block_hash;
            ar & (uint8_t&)block_type;
        }
    };

    struct BlockHeader {
        GenericHeader generic_header;
        uint64_t block_uid;

        BlockHeader()
        : generic_header()
        , block_uid(0)
        {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & generic_header;
            ar & block_uid;
        }
    };

    struct CollectionBlock;
    struct BaselineBlock;

    struct BaseBlock {
        BlockHeader header;

        BaseBlock()
        : header()
        {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & header;
        }
    };

    struct SubBlockHeader {
        GenericHeader generic_header;

        SubBlockHeader()
        : generic_header()
        {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & generic_header;
        }
    };

    struct MiningState {
        epoch_t epoch;
        hash_t highest_hash_of_last_epoch;
        hash_t highest_hash_of_current_epoch;
        int32_t num_minings_in_epoch;

        MiningState()
        : epoch(0)
        , highest_hash_of_last_epoch(0)
        , highest_hash_of_current_epoch(0)
        , num_minings_in_epoch(0)
        {}

        template<class Archive>
        void ser(Archive& ar) {
            ar & epoch;
            ar & highest_hash_of_last_epoch;
            ar & highest_hash_of_current_epoch;
            ar & num_minings_in_epoch;
        }
    };

    struct BaselineBlock : public BaseBlock {
        MiningState mining_state;
        std::vector<std::vector<hash_t>> data_value_hashes; //vector of vectors of all existing data value hashes - 1st index: epoch, 2nd index: data_value in epoch
        std::map<public_key_t, uint64_t> wallets;

        BaselineBlock()
        : mining_state()
        , data_value_hashes()
        , wallets() {
            header.generic_header.block_type = BlockType::BaselineBlock;
        }

        template<class Archive>
        void ser(Archive& ar) {
            ar & cereal::base_class<BaseBlock>( this );
            ar & mining_state;
            ar & data_value_hashes;
            ar & wallets;
        }
    };

    struct TransactionSubBlock {
        SubBlockHeader header;
        uint64_t fraction;
        public_key_t pre_owner;
        public_key_t post_owner;
        signature_t signature;

        static const uint32_t fraction_per_coin = 1000000;

        TransactionSubBlock()
        : header()
        , fraction(0)
        , pre_owner()
        , post_owner()
        , signature() {
            header.generic_header.block_type = BlockType::TransactionSubBlock;
        }

        template<class Archive>
        void ser(Archive& ar) {
            ar & header;
            ar & fraction;
            ar & pre_owner;
            ar & post_owner;
            ar & signature;
        }
    };

    struct CreationSubBlock {
        SubBlockHeader header;
        std::string data_value;
        public_key_t creator;
        signature_t signature;

        static const uint32_t max_data_value_length = 1024;

        CreationSubBlock()
        : header()
        , data_value()
        , creator()
        , signature() {
            header.generic_header.block_type = BlockType::CreationSubBlock;
        }

        template<class Archive>
        void ser(Archive& ar) {
            ar & header;
            ar & data_value;
            ar & creator;
            ar & signature;
        }
    };

    struct CollectionBlock: public BaseBlock {
        std::map<hash_t, TransactionSubBlock> transactions;
        std::map<hash_t, CreationSubBlock> creations;

        static const uint32_t max_num_creations    = 1000;
        static const uint32_t max_num_transactions = 10000;

        CollectionBlock()
        : transactions()
        , creations() {
            header.generic_header.block_type = BlockType::CollectionBlock;
        }

        template<class Archive>
        void ser(Archive& ar) {
            ar & cereal::base_class<BaseBlock>( this );
            ar & transactions;
            ar & creations;
        }
    };

    std::ostream& operator<<(std::ostream& os, const BaseBlock& block);
    std::ostream& operator<<(std::ostream& os, const BaselineBlock& block);
    std::ostream& operator<<(std::ostream& os, const CollectionBlock& block);
    std::ostream& operator<<(std::ostream& os, const TransactionSubBlock& block);
    std::ostream& operator<<(std::ostream& os, const CreationSubBlock& block);
}

#endif //FULL_NODE_BLOCKDEFINITIONS_H
