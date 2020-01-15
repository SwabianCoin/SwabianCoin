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

#ifndef FULL_NODE_CACHE_H
#define FULL_NODE_CACHE_H

#include "scn/Common/Common.h"
#include "BlockDefinitions.h"
#include <mutex>
#include <thread>
#include <map>

namespace scn {

    class Cache {
    public:

        Cache(const std::string& folder_path);

        virtual ~Cache();

        virtual const std::shared_ptr<BaseBlock> getBlock(const block_uid_t uid) const;

        virtual block_uid_t addBlock(const BaselineBlock& block);

        virtual block_uid_t addBlock(const CollectionBlock& block);

        virtual void resetCache(const uint64_t root_block_uid);

        static const std::shared_ptr<BaseBlock> getExternalBlockFromDisk(const std::string& folder_path, const block_uid_t uid);

    protected:

        static const uint32_t target_cache_size = 10;

        virtual void cacheThread();

        virtual const std::shared_ptr<BaseBlock> getBlockFromDisk(const block_uid_t uid) const;

        virtual void writeBlockToDisk(const BaselineBlock& block);

        virtual void writeBlockToDisk(const CollectionBlock& block);

        const std::string folder_path_;

        block_uid_t next_free_block_id_;

        mutable std::mutex mtx_cache_access_;
        std::map<block_uid_t, std::shared_ptr<BaseBlock>> cached_blocks_;
        mutable std::mutex mtx_hd_access_;
        mutable std::mutex mtx_cache_hd_transfer_;

        bool running_;
        std::thread cache_thread_;

    };

}


#endif //FULL_NODE_CACHE_H
