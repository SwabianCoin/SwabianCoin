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

#include "Cache.h"
#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <boost/filesystem.hpp>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace scn;


Cache::Cache(const std::string& folder_path)
:folder_path_(folder_path)
,next_free_block_id_(1)
,running_(true)
,cache_thread_(&Cache::cacheThread, this) {
    boost::filesystem::create_directories(folder_path_);

#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
#else
    struct sched_param p;
    p.sched_priority=0;
    pthread_setschedparam(cache_thread_.native_handle(),SCHED_IDLE,&p);
#endif
}


Cache::~Cache() {
    running_ = false;
    cache_thread_.join();
}


const std::shared_ptr<BaseBlock> Cache::getBlock(const block_uid_t uid) const {
    {
        LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
        auto cached_block = cached_blocks_.find(uid);
        if(cached_block != cached_blocks_.end()) {
            return cached_block->second;
        }
    }
    return getBlockFromDisk(uid);
}


block_uid_t Cache::addBlock(const BaselineBlock& block) {
    auto this_block_id = next_free_block_id_;
    assert(this_block_id == block.header.block_uid);
    auto block_to_cache = std::make_shared<BaselineBlock>(block);
    {
        LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
        cached_blocks_[this_block_id] = block_to_cache;
    }
    next_free_block_id_++;
    return this_block_id;
}


block_uid_t Cache::addBlock(const CollectionBlock& block) {
    auto this_block_id = next_free_block_id_;
    assert(this_block_id == block.header.block_uid);
    auto block_to_cache = std::make_shared<CollectionBlock>(block);
    {
        LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
        cached_blocks_[this_block_id] = block_to_cache;
    }
    next_free_block_id_++;
    return this_block_id;
}


const std::shared_ptr<BaseBlock> Cache::getBlockFromDisk(const block_uid_t uid) const {
    if(uid == 0)
    {
        return nullptr;
    }

    try {
        LOCK_MUTEX_WATCHDOG(mtx_hd_access_);
        BlockType block_type;
        std::ifstream ifs(folder_path_ + "/" + std::to_string(uid) + ".blk", std::ifstream::binary);
        cereal::PortableBinaryInputArchive ia(ifs);
        ia >> block_type;

        switch (block_type) {
            case BlockType::BaselineBlock: {
                LOG(INFO) << "Reading baseline block from disk...";
                auto block = std::make_shared<BaselineBlock>();
                ia >> *block;
                LOG(INFO) << "Finished reading baseline block from disk";
                return std::static_pointer_cast<BaseBlock>(block);
                break;
            }
            case BlockType::CollectionBlock: {
                auto block = std::make_shared<CollectionBlock>();
                ia >> *block;
                return std::static_pointer_cast<BaseBlock>(block);
                break;
            }
            default:
                assert(false); //should never happen
                break;
        }
    }
    catch(const std::exception& e) {
        return nullptr;
    }
    return nullptr;
}


const std::shared_ptr<BaseBlock> Cache::getExternalBlockFromDisk(const std::string& folder_path, const block_uid_t uid) {
    if(uid == 0)
    {
        return nullptr;
    }

    try {
        BlockType block_type;
        std::ifstream ifs(folder_path + "/" + std::to_string(uid) + ".blk", std::ifstream::binary);
        cereal::PortableBinaryInputArchive ia(ifs);
        ia >> block_type;

        switch (block_type) {
            case BlockType::BaselineBlock: {
                auto block = std::make_shared<BaselineBlock>();
                ia >> *block;
                return std::static_pointer_cast<BaseBlock>(block);
                break;
            }
            case BlockType::CollectionBlock: {
                auto block = std::make_shared<CollectionBlock>();
                ia >> *block;
                return std::static_pointer_cast<BaseBlock>(block);
                break;
            }
            default:
                assert(false); //should never happen
                break;
        }
    }
    catch(const std::exception& e) {
        return nullptr;
    }
    return nullptr;
}


void Cache::writeBlockToDisk(const BaselineBlock& block) {
    LOCK_MUTEX_WATCHDOG(mtx_hd_access_);
    LOG(INFO) << "Writing baseline block from cache to disk...";
    std::ofstream ofs(folder_path_ + "/" + std::to_string(block.header.block_uid) + ".blk", std::ofstream::binary);
    cereal::PortableBinaryOutputArchive oa(ofs);
    oa << (uint8_t)block.header.generic_header.block_type;
    oa << block;
    LOG(INFO) << "Finished writing baseline block from cache to disk";
}


void Cache::writeBlockToDisk(const CollectionBlock& block) {
    LOCK_MUTEX_WATCHDOG(mtx_hd_access_);
    std::ofstream ofs(folder_path_ + "/" + std::to_string(block.header.block_uid) + ".blk", std::ofstream::binary);
    cereal::PortableBinaryOutputArchive oa(ofs);
    oa << (uint8_t)block.header.generic_header.block_type;
    oa << block;
}


void Cache::resetCache(const uint64_t root_block_uid) {
    {
        LOCK_MUTEX_WATCHDOG(mtx_cache_hd_transfer_);
        {
            LOCK_MUTEX_WATCHDOG(mtx_hd_access_);
            //remove all blocks with smaller or equal block_id
            for (block_uid_t i = 1; i <= root_block_uid; i++) {
                boost::filesystem::remove(folder_path_ + "/" + std::to_string(i) + ".blk");
            }
        }

        {
            LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
            cached_blocks_.clear();
        }
    }

    //add block with given id
    next_free_block_id_ = root_block_uid;
}


void Cache::cacheThread() {
    while(running_) {

        {
            LOCK_MUTEX_WATCHDOG(mtx_cache_hd_transfer_);
            std::shared_ptr<scn::BaseBlock> block_to_transfer = nullptr;
            {
                LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
                if (cached_blocks_.size() > target_cache_size) {
                    block_to_transfer = cached_blocks_.begin()->second;
                }
            }

            if (block_to_transfer) {
                switch (block_to_transfer->header.generic_header.block_type) {
                    case BlockType::BaselineBlock: {
                        auto block = std::static_pointer_cast<scn::BaselineBlock>(block_to_transfer);
                        writeBlockToDisk(*block);
                        break;
                    }
                    case BlockType::CollectionBlock: {
                        auto block = std::static_pointer_cast<scn::CollectionBlock>(block_to_transfer);
                        writeBlockToDisk(*block);
                        break;
                    }
                    default:
                        assert(false); //should never happen
                        break;
                }
            }

            if (block_to_transfer) {
                LOCK_MUTEX_WATCHDOG(mtx_cache_access_);
                cached_blocks_.erase(block_to_transfer->header.block_uid);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
}