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

#ifndef FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H
#define FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H

#include "ICycleState.h"
#include <array>
#include <map>
#include <set>


namespace scn {

    class BlockchainManager;

    class CycleStateIntroduceBlock : public ICycleState {
    public:
        CycleStateIntroduceBlock(BlockchainManager& base);

        virtual ~CycleStateIntroduceBlock();

        virtual void onEnter() override;

        virtual bool onCycle() override;

        virtual void onExit() override;

        virtual void blockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const CollectionBlock> block, bool reply) override;

        virtual State getState() const override { return State::IntroduceBlock; }

        static const blockchain_time_t time_between_propagations_ms_ = 4000;

    protected:

        void getRidOfDuplicates(std::map<hash_t, CreationSubBlock>& map_to_modify, const std::string& data_value_to_check);

        BlockchainManager& base_;
        blockchain_time_t next_propagation_time_;
        uint32_t num_propagations_in_current_cycle_;

        std::map<hash_t, bool> processed_blocks_; //2nd parameter: valid/invalid

        std::map<hash_t, uint32_t> creation_sub_block_counter;
        std::map<hash_t, uint32_t> transaction_sub_block_counter;

        static const std::array<uint32_t, 22> peers_necessary_for_granting_;
    };

}

#endif //FULL_NODE_CYCLESTATEINTRODUCEBLOCK_H
