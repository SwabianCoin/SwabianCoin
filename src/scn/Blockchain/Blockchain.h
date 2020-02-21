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

#ifndef FULL_NODE_BLOCKCHAIN_H
#define FULL_NODE_BLOCKCHAIN_H

#include "scn/Common/Common.h"
#include "BlockDefinitions.h"
#include "Cache.h"
#include "scn/CryptoHelper/CryptoHelper.h"
#include <mutex>

namespace scn {

    class Blockchain {
    public:

        explicit Blockchain(const std::string& folder_path);

        virtual ~Blockchain();

        virtual void initEmptyChain();

        virtual void importBlockchain(const std::string& folder_path);

        virtual const std::shared_ptr<BaseBlock> getBlock(block_uid_t uid) const;

        virtual const std::shared_ptr<BaseBlock> getRootBlock() const;

        virtual const std::shared_ptr<BaseBlock> getNewestBlock() const;

        virtual block_uid_t getRootBlockId() const;

        virtual block_uid_t getNewestBlockId() const;

        virtual block_uid_t addBlock(const BaselineBlock& block);

        virtual block_uid_t addBlock(const CollectionBlock& block);

        virtual void setRootBlock(const BaselineBlock& block);

        virtual block_uid_t establishBaseline();

        virtual uint64_t getBalance(const public_key_t& public_key);

        virtual uint64_t getNumWallets() const;

        virtual MiningState getMiningState() const;

        bool validateBlock(const BaselineBlock& block);

        static bool validateBlockWithoutContext(const BaselineBlock& block);

        bool validateBlock(const CollectionBlock& block);

        bool validateSubBlock(const TransactionSubBlock& sub_block);

        bool validateSubBlock(const CreationSubBlock& sub_block);

        static void getHashArea(const epoch_t& epoch, hash_t& max_allowed_hash, hash_t& min_allowed_hash);

        void writeCurrentBaselineToFile(const std::string& filename);

    protected:

        struct MetaData {
            block_uid_t root_block_id;
            block_uid_t newest_block_id;

            template<class Archive>
            void ser(Archive& ar)
            {
                ar & root_block_id;
                ar & newest_block_id;
            }
        };

        bool validateSubBlock(const TransactionSubBlock& sub_block, BaseBlock& newest_block_in_chain);

        bool validateSubBlock(const CreationSubBlock& sub_block,
                              BaseBlock& newest_block_in_chain,
                              MiningState& mining_state,
                              hash_t& max_allowed_hash,
                              hash_t& min_allowed_hash,
                              std::vector<hash_t>& data_value_hashes_of_epoch);

        virtual MetaData getMetaData() const;

        static MetaData getExternalMetaData(const std::string& folder_path);

        virtual void setMetaData(MetaData& meta);

        void updateCurrentBaseline(const CollectionBlock& block);

        Cache cache_;
        const std::string folder_path_;

        mutable std::mutex mtx_current_baseline_access_;
        BaselineBlock current_baseline_;

        mutable MetaData current_meta_data_;
        bool current_meta_data_initialized_;
    };

    std::ostream& operator<<(std::ostream& os, const Blockchain& blockchain);
}

#endif //FULL_NODE_BLOCKCHAIN_H
