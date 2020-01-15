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

#include "Blockchain.h"
#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <set>

using namespace scn;


Blockchain::Blockchain(const std::string& folder_path)
:cache_(folder_path)
,folder_path_(folder_path)
,current_meta_data_initialized_(false) {

}


Blockchain::~Blockchain() {

}


void Blockchain::initEmptyChain()
{
    BaselineBlock empty_baseline;
    empty_baseline.mining_state.epoch = 0;
    empty_baseline.mining_state.highest_hash_of_last_epoch = 0;
    empty_baseline.mining_state.highest_hash_of_current_epoch = 0;
    empty_baseline.mining_state.num_minings_in_epoch = 0;
    empty_baseline.wallets.clear();
    empty_baseline.data_value_hashes.clear();
    empty_baseline.data_value_hashes.emplace_back();
    empty_baseline.header.block_uid = 1;
    empty_baseline.header.generic_header.block_type = BlockType::BaselineBlock;
    empty_baseline.header.generic_header.previous_block_hash = 0;
    CryptoHelper::fillHash(empty_baseline);
    setRootBlock(empty_baseline);
}

void Blockchain::importBlockchain(const std::string& folder_path) {
    auto meta_data = getExternalMetaData(folder_path);
    if(meta_data.root_block_id == 0 || meta_data.newest_block_id == 0) {
        LOG(ERROR) << "Could not import external blockchain";
        initEmptyChain();
        return;
    }

    auto baseline_block = std::static_pointer_cast<scn::BaselineBlock>(Cache::getExternalBlockFromDisk(folder_path, meta_data.root_block_id));
    setRootBlock(*baseline_block);

    for(block_uid_t i=meta_data.root_block_id+1;i<=meta_data.newest_block_id;i++) {
        auto collection_block = std::static_pointer_cast<scn::CollectionBlock>(Cache::getExternalBlockFromDisk(folder_path, i));
        addBlock(*collection_block);
    }
}

const std::shared_ptr<BaseBlock> Blockchain::getBlock(const block_uid_t uid) const {
    return cache_.getBlock(uid);
}

const std::shared_ptr<BaseBlock> Blockchain::getRootBlock() const {
    MetaData meta = getMetaData();
    return getBlock(meta.root_block_id);
}

const std::shared_ptr<BaseBlock> Blockchain::getNewestBlock() const {
    MetaData meta = getMetaData();
    return getBlock(meta.newest_block_id);
}

block_uid_t Blockchain::getRootBlockId() const {
    MetaData meta = getMetaData();
    return meta.root_block_id;
}


block_uid_t Blockchain::getNewestBlockId() const {
    MetaData meta = getMetaData();
    return meta.newest_block_id;
}


block_uid_t Blockchain::addBlock(const BaselineBlock& block) {
    auto this_block_id = cache_.addBlock(block);
    {
        LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
        current_baseline_ = block;
        current_baseline_.header.generic_header.block_hash = 0;
    }

    MetaData meta = getMetaData();
    meta.newest_block_id = this_block_id;
    setMetaData(meta);
    
    return this_block_id;
}


block_uid_t Blockchain::addBlock(const CollectionBlock& block) {
    auto this_block_id = cache_.addBlock(block);
    
    MetaData meta = getMetaData();
    meta.newest_block_id = this_block_id;
    setMetaData(meta);

    updateCurrentBaseline(block);

    return this_block_id;
}


void Blockchain::setRootBlock(const BaselineBlock& block) {
    if(block.header.block_uid == 0)
    {
        return;
    }

    cache_.resetCache(block.header.block_uid);
    addBlock(block);

    //add meta file
    {
        MetaData meta;
        meta.root_block_id = block.header.block_uid;
        meta.newest_block_id = block.header.block_uid;
        setMetaData(meta);
    }
}


block_uid_t Blockchain::establishBaseline() {
    block_uid_t this_block_id;
    {
        LOG(INFO) << "establishBaseline()";
        google::FlushLogFiles(google::GLOG_INFO);
        LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
        LOG(INFO) << "  got mutex";
        google::FlushLogFiles(google::GLOG_INFO);
        cache_.resetCache(current_baseline_.header.block_uid);
        LOG(INFO) << "  reset cache done";
        google::FlushLogFiles(google::GLOG_INFO);
        CryptoHelper::fillHash(current_baseline_);
        LOG(INFO) << "  filled hash";
        google::FlushLogFiles(google::GLOG_INFO);
        this_block_id = cache_.addBlock(current_baseline_);
        LOG(INFO) << "New Baseline " << current_baseline_.header.block_uid << ": " << current_baseline_.header.generic_header.block_hash.str(0, std::ios_base::hex);
        google::FlushLogFiles(google::GLOG_INFO);
        current_baseline_.header.generic_header.block_hash = 0;

        //add meta file
        {
            MetaData meta;
            meta.root_block_id = current_baseline_.header.block_uid;
            meta.newest_block_id = current_baseline_.header.block_uid;
            setMetaData(meta);
        }
    }
    return this_block_id;
}


uint64_t Blockchain::getBalance(const public_key_t& public_key) {
    {
        LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
        auto it = current_baseline_.wallets.find(public_key);
        if(it == current_baseline_.wallets.end()) {
            return 0;
        } else {
            return it->second;
        }
    }
}


uint64_t Blockchain::getNumWallets() const {
    LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
    return current_baseline_.wallets.size();
}


MiningState Blockchain::getMiningState() const {
    LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
    return current_baseline_.mining_state;
}


bool Blockchain::validateBlock(const BaselineBlock& block) {
    //check if block is valid and fits to new block in current blockchain

    if(!validateBlockWithoutContext(block)) {
        return false;
    }

    //check previous block hash
    auto newest_block_in_chain = getNewestBlock();
    if(block.header.generic_header.previous_block_hash != newest_block_in_chain->header.generic_header.block_hash) {
        LOG(ERROR) << "validateBlock: previous block hash invalid";
        return false;
    }

    //check block id
    if(block.header.block_uid != newest_block_in_chain->header.block_uid + 1) {
        LOG(ERROR) << "validateBlock: block uid invalid: " << block.header.block_uid << " - expected: " << newest_block_in_chain->header.block_uid + 1;
        return false;
    }

    return true;
}


bool Blockchain::validateBlockWithoutContext(const BaselineBlock& block) {
    //check if block is valid

    //check block type
    if(block.header.generic_header.block_type != BlockType::BaselineBlock) {
        LOG(ERROR) << "validateBlock: block type invalid";
        return false;
    }

    //check block id for zero
    if(block.header.block_uid == 0) {
        LOG(ERROR) << "validateBlock: block uid invalid: " << block.header.block_uid;
        return false;
    }

    //check block hash
    if(!CryptoHelper::verifyHash(block)) {
        LOG(ERROR) << "validateBlock: block hash invalid";
        return false;
    }

    //check wallets for mismatch between sum of balances and sum of data value hashes
    uint64_t wallets_sum = 0, data_value_hashes_sum = 0;
    for(auto& wallet : block.wallets) {
        wallets_sum += wallet.second;
    }
    for(auto& epoch_hashes : block.data_value_hashes) {
        data_value_hashes_sum += epoch_hashes.size() * TransactionSubBlock::fraction_per_coin;
    }
    if(data_value_hashes_sum != wallets_sum) {
        LOG(ERROR) << "validateBlock: data_value_hashes sum does not match wallets sum: " << data_value_hashes_sum << " " << wallets_sum;
        return false;
    }

    //check wallets for invalid public keys
    for(auto& wallet : block.wallets) {
        if(!CryptoHelper::isPublicKeyValid(wallet.first)) {
            LOG(ERROR) << "validateBlock: found invalid wallet public key:  " << wallet.first.getAsShortString();
            return false;
        }
    }

    //check data_value_hashes vector of vectors for correct size (1st level)
    if(block.data_value_hashes.size() != block.mining_state.epoch+1) {
        LOG(ERROR) << "validateBlock: data_value_hashes size does not match epoch";
        return false;
    }

    //check data_value_hashes vector of vectors for correct size (2nd level)
    for(auto it = block.data_value_hashes.begin() ; it != block.data_value_hashes.end() ; it++) {
        if(it == std::prev(block.data_value_hashes.end())) {
            if(it->size() > CollectionBlock::max_num_creations) {
                LOG(ERROR) << "validateBlock: too many creations in last epoch of data_value_hashes: " << it->size();
                return false;
            }
        } else {
            if(it->size() != CollectionBlock::max_num_creations) {
                LOG(ERROR) << "validateBlock: invalid number of creations in epoch of data_value_hashes: " << it->size();
                return false;
            }
        }
    }

    //check data_value_hashes vector of vectors for correct sorting
    for(auto& epoch_hashes : block.data_value_hashes) {
        if(!std::is_sorted(epoch_hashes.begin(), epoch_hashes.end())) {
            LOG(ERROR) << "validateBlock: data_value_hashes not sorted";
            return false;
        }
    }

    //check data_value_hashes for double entries
    for(auto& epoch_hashes : block.data_value_hashes) {
        for(auto it = epoch_hashes.begin() ; it != epoch_hashes.end() ; it++) {
            if((it+1) != epoch_hashes.end() && *it == *(it+1)) {
                LOG(ERROR) << "validateBlock: duplicate data_value hash found: " << it->str(0, std::ios_base::hex);
                return false;
            }
        }
    }

    //check data_value_hashes vector of vectors for correct hash area
    hash_t max_allowed_hash, min_allowed_hash;
    for(uint32_t epoch = 0 ; epoch < block.data_value_hashes.size() ; epoch++) {
        getHashArea(epoch, max_allowed_hash, min_allowed_hash);
        if(block.data_value_hashes[epoch].size() > 0) {
            if (block.data_value_hashes[epoch].front() < min_allowed_hash ||
                block.data_value_hashes[epoch].front() > max_allowed_hash ||
                block.data_value_hashes[epoch].back() < min_allowed_hash ||
                block.data_value_hashes[epoch].back() > max_allowed_hash) {
                LOG(ERROR) << "validateBlock: data_value_hash outside allowed hash area";
                return false;
            }
        }
    }

    //check mining state for correct highest hash of current epoch
    if(block.data_value_hashes.size() == 0 || block.data_value_hashes.back().size() == 0) {
        if(block.mining_state.highest_hash_of_current_epoch != 0) {
            LOG(ERROR) << "validateBlock: highest_hash_of_current_epoch does not match expected value 0";
            return false;
        }
    } else if(block.mining_state.highest_hash_of_current_epoch != block.data_value_hashes.back().back()) {
        LOG(ERROR) << "validateBlock: highest_hash_of_current_epoch does not match expected value ";
        return false;
    }

    //check mining state for correct highest hash of last epoch
    if(block.data_value_hashes.size() <= 1) {
        if(block.mining_state.highest_hash_of_last_epoch != 0) {
            LOG(ERROR) << "validateBlock: highest_hash_of_last_epoch does not match expected value 0";
            return false;
        }
    } else if(block.mining_state.highest_hash_of_last_epoch != (block.data_value_hashes.end()-2)->back()) {
        LOG(ERROR) << "validateBlock: highest_hash_of_last_epoch does not match expected value ";
        return false;
    }

    //check mining state for correct number of minings
    if(block.data_value_hashes.size() == 0) {
        if(block.mining_state.num_minings_in_epoch != 0) {
            LOG(ERROR) << "validateBlock: num_minings_in_epoch does not match expected value 0";
            return false;
        }
    } else if(block.mining_state.num_minings_in_epoch != static_cast<int32_t>(block.data_value_hashes.back().size())) {
        LOG(ERROR) << "validateBlock: num_minings_in_epoch does not match expected value";
        return false;
    }

    return true;
}


bool Blockchain::validateBlock(const CollectionBlock& block) {
    //check if block is valid and fits to new block in current blockchain

    std::chrono::time_point<std::chrono::system_clock> t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    t0 = std::chrono::system_clock::now();

    //check block id
    auto newest_block_in_chain = getNewestBlock();
    if(block.header.block_uid == 0 || block.header.block_uid != newest_block_in_chain->header.block_uid + 1) {
        LOG(ERROR) << "validateBlock: block uid invalid: " << block.header.block_uid << " - expected: " << newest_block_in_chain->header.block_uid + 1;
        return false;
    }

    //check block type
    if(block.header.generic_header.block_type != BlockType::CollectionBlock) {
        LOG(ERROR) << "validateBlock: block type invalid";
        return false;
    }

    t1 = std::chrono::system_clock::now();

    //check block hash
    if(!CryptoHelper::verifyHash(block)) {
        LOG(ERROR) << "validateBlock: block hash invalid";
        return false;
    }

    t2 = std::chrono::system_clock::now();

    //check every transaction
    for(auto& transaction : block.transactions) {
        if(!validateSubBlock(transaction.second, *newest_block_in_chain)) {
            LOG(ERROR) << "validateBlock: transaction invalid";
            return false;
        }
    }

    t3 = std::chrono::system_clock::now();

    //check every creation
    auto mining_state = getMiningState();
    hash_t max_allowed_hash, min_allowed_hash;
    getHashArea(mining_state.epoch, max_allowed_hash, min_allowed_hash);
    std::vector<hash_t> data_value_hashes_of_epoch;
    {
        LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
        if(current_baseline_.data_value_hashes.size() > 0) {
            data_value_hashes_of_epoch = current_baseline_.data_value_hashes.back();
        }
    }
    for(auto& creation : block.creations) {
        if(!validateSubBlock(creation.second,
                             *newest_block_in_chain,
                             mining_state,
                             max_allowed_hash,
                             min_allowed_hash,
                             data_value_hashes_of_epoch)) {
            LOG(ERROR) << "validateBlock: creation invalid";
            return false;
        }
    }

    t4 = std::chrono::system_clock::now();

    //check sorting of creations
    hash_t last_hash = 0;
    for(auto& creation : block.creations) {
        if(creation.second.header.generic_header.block_hash < last_hash) {
            LOG(ERROR) << "validateBlock: creations sorting invalid";
            return false;
        }
        last_hash = creation.second.header.generic_header.block_hash;
    }

    t5 = std::chrono::system_clock::now();

    //check if number of creations left is exceeded
    if(block.creations.size() > (CollectionBlock::max_num_creations - mining_state.num_minings_in_epoch)) {
        LOG(ERROR) << "validateBlock: creation number invalid";
        return false;
    }

    //check if number of transactions is exceeded
    if(block.transactions.size() > CollectionBlock::max_num_transactions) {
        LOG(ERROR) << "validateBlock: transaction number invalid";
        return false;
    }

    t6 = std::chrono::system_clock::now();

    //check previous block hash
    if(block.header.generic_header.previous_block_hash != newest_block_in_chain->header.generic_header.block_hash) {
        LOG(ERROR) << "validateBlock: previous block hash invalid";
        return false;
    }

    t7 = std::chrono::system_clock::now();

    //check that every data_value is unique in creations map
    std::set<std::string> data_value_hashes;
    for(auto& creation : block.creations) {
        auto return_value = data_value_hashes.insert(creation.second.data_value);
        if(!return_value.second) {
            LOG(ERROR) << "validateBlock: duplicate data_value in creation map";
            return false;
        }
    }

    t8 = std::chrono::system_clock::now();

    //check that every wallet's balance is >= 0
    {
        //calculate resulting balance for all wallets which have creations or transactions in this block
        std::map<public_key_t, int64_t> wallet_diffs; //NOTE: use signed integer here to detect negative balance values
        for(auto& creation : block.creations) {
            wallet_diffs[creation.second.creator] += TransactionSubBlock::fraction_per_coin;
        }
        for(auto& transaction : block.transactions) {
            wallet_diffs[transaction.second.pre_owner] -= transaction.second.fraction;
            wallet_diffs[transaction.second.post_owner] += transaction.second.fraction;
        }
        {
            LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
            for (auto& wallet_diff : wallet_diffs) {
                if (wallet_diff.second < 0 && wallet_diff.second + static_cast<int64_t>(current_baseline_.wallets[wallet_diff.first]) < 0) {
                    LOG(ERROR) << "validateBlock: wallet's balance is invalid: "
                               << wallet_diff.second + static_cast<int64_t>(current_baseline_.wallets[wallet_diff.first])
                               << " - " << wallet_diff.first.getAsShortString();
                    return false;
                }
            }
        }
    }

    t9 = std::chrono::system_clock::now();

    LOG(INFO) << "Blockchain::validateBlock(const CollectionBlock& block) t01:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)).count() << " t12:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count() << " t23:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2)).count() << " t34:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3)).count() << " t45:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4)).count() << " t56:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5)).count() << " t67:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6)).count() << " t78:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t8 - t7)).count() << " t89:" <<
              (std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t8)).count();

    return true;
}


bool Blockchain::validateSubBlock(const TransactionSubBlock& sub_block) {
    return validateSubBlock(sub_block, *getNewestBlock());
}


bool Blockchain::validateSubBlock(const TransactionSubBlock& sub_block, BaseBlock& newest_block_in_chain) {
    //check if block is valid and fits to new block in current blockchain

    //check for valid fraction (zero transactions are not allowed)
    if(sub_block.fraction == 0) {
        LOG(ERROR) << "validateSubBlock: fraction invalid (zero transactions are not allowed)";
        return false;
    }

    //check block type
    if(sub_block.header.generic_header.block_type != BlockType::TransactionSubBlock) {
        LOG(ERROR) << "validateSubBlock: block type invalid";
        return false;
    }

    //check block hash
    if(!CryptoHelper::verifyHash(sub_block)) {
        LOG(ERROR) << "validateSubBlock: block hash invalid";
        return false;
    }

    //check signature with pre_owner public key
    if(!CryptoHelper::verifySignature(sub_block, sub_block.pre_owner)) {
        LOG(ERROR) << "validateSubBlock: signature invalid";
        return false;
    }

    //check that pre_owner and post_owner are not the same
    if(sub_block.pre_owner == sub_block.post_owner) {
        LOG(ERROR) << "validateSubBlock: pre_owner and post_owner are equal";
        return false;
    }

    //check post_owner for validness (is it a valid key?)
    if(!CryptoHelper::isPublicKeyValid(sub_block.post_owner)) {
        LOG(ERROR) << "validateSubBlock: post_owner is invalid:  " << sub_block.post_owner.getAsShortString();
        return false;
    }

    //check previous block hash
    if(sub_block.header.generic_header.previous_block_hash != newest_block_in_chain.header.generic_header.block_hash) {
        LOG(ERROR) << "validateSubBlock: previous block hash invalid";
        return false;
    }

    return true;
}


bool Blockchain::validateSubBlock(const CreationSubBlock& sub_block) {
    auto mining_state = getMiningState();
    hash_t max_allowed_hash, min_allowed_hash;
    getHashArea(mining_state.epoch, max_allowed_hash, min_allowed_hash);
    std::vector<hash_t> data_value_hashes_of_epoch;
    {
        LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
        if(current_baseline_.data_value_hashes.size() > 0) {
            data_value_hashes_of_epoch = current_baseline_.data_value_hashes.back();
        }
    }
    return validateSubBlock(sub_block,
                            *getNewestBlock(),
                            mining_state,
                            max_allowed_hash,
                            min_allowed_hash,
                            data_value_hashes_of_epoch);
}


bool Blockchain::validateSubBlock(const CreationSubBlock& sub_block,
                                  BaseBlock& newest_block_in_chain,
                                  MiningState& mining_state,
                                  hash_t& max_allowed_hash,
                                  hash_t& min_allowed_hash,
                                  std::vector<hash_t>& data_value_hashes_of_epoch) {
    //check if block is valid and fits to new block in current blockchain

    //check block type
    if(sub_block.header.generic_header.block_type != BlockType::CreationSubBlock) {
        LOG(ERROR) << "validateSubBlock: block type invalid";
        return false;
    }

    //check block hash
    if(!CryptoHelper::verifyHash(sub_block)) {
        LOG(ERROR) << "validateSubBlock: block hash invalid";
        return false;
    }

    //check signature with creator public key
    if(!CryptoHelper::verifySignature(sub_block, sub_block.creator)) {
        LOG(ERROR) << "validateSubBlock: signature invalid";
        return false;
    }

    //check data value maximum length
    if(sub_block.data_value.length() > CreationSubBlock::max_data_value_length) {
        LOG(ERROR) << "validateSubBlock: Data value maximum length exceeded: " << sub_block.data_value.length();
        return false;
    }

    //check data value content (begins with public key and previous hash)
    if( !boost::starts_with(sub_block.data_value,
                            mining_state.highest_hash_of_last_epoch.str(0, std::ios_base::hex) + "_" +
                                    sub_block.creator.getAsShortString() + "_")) {
        LOG(ERROR) << "validateSubBlock: data_value beginning invalid" /*<< std::endl
                   << "expected: " << mining_state.highest_hash_of_last_epoch.str(0, std::ios_base::hex) + "_" + sub_block.creator + "_" << std::endl
                   << "found:    " << sub_block.data_value*/;
        return false;
    }

    //check data value hash area
    hash_t data_value_hash = CryptoHelper::calcHash(sub_block.data_value);
    if(data_value_hash < min_allowed_hash || data_value_hash > max_allowed_hash) {
        LOG(ERROR) << "validateSubBlock: Data value hash area mismatch!" << std::endl << data_value_hash.str(0, std::ios_base::hex) << std::endl << sub_block.data_value << std::endl << "min/max:" << min_allowed_hash.str(0, std::ios_base::hex) << "/" << max_allowed_hash.str(0, std::ios_base::hex);
        return false;
    }

    //check previous block hash
    if(sub_block.header.generic_header.previous_block_hash != newest_block_in_chain.header.generic_header.block_hash) {
        LOG(ERROR) << "validateSubBlock: previous block hash invalid";
        return false;
    }

    //check if data_value is already in blockchain
    if(std::binary_search(data_value_hashes_of_epoch.begin(), data_value_hashes_of_epoch.end(), data_value_hash)) {
        LOG(ERROR) << "validateSubBlock: data_value already found in blockchain: " << data_value_hash.str(0, std::ios_base::hex);
        return false;
    }

    return true;
}


void Blockchain::getHashArea(const epoch_t& epoch, hash_t& max_allowed_hash, hash_t& min_allowed_hash) {
    boost::multiprecision::uint512_t temp = std::numeric_limits<boost::multiprecision::uint256_t>::max();
    for(epoch_t i=0;i<epoch;i++)
    {
        temp = temp *
               boost::multiprecision::uint512_t(1000000000000000ul) /
               boost::multiprecision::uint512_t(1001776032062287ul);
    }
    max_allowed_hash = boost::multiprecision::uint256_t(temp);
    min_allowed_hash = std::min(boost::multiprecision::uint256_t(
            boost::multiprecision::uint512_t(max_allowed_hash) *
            boost::multiprecision::uint512_t(1000000000000000ul) /
            boost::multiprecision::uint512_t(1001776032062287ul)) + 1, max_allowed_hash);
}


void Blockchain::writeDataValueHashesToFile(const std::string& filename) {
    LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
    std::ofstream ofs(filename);
    for(epoch_t epoch=0;epoch<current_baseline_.data_value_hashes.size();epoch++) {
        ofs << "Epoch: " << epoch << ": " << current_baseline_.data_value_hashes[epoch].size() << std::endl;
    }
    for(auto& hash : current_baseline_.data_value_hashes.back()) {
        ofs << "  " << hash.str(0, std::ios_base::hex) << std::endl;
    }
}


Blockchain::MetaData Blockchain::getMetaData() const {
    if(!current_meta_data_initialized_) {
        try {
            std::ifstream ifs(folder_path_ + "/meta");
            cereal::PortableBinaryInputArchive ia(ifs);
            ia >> current_meta_data_;
        } catch (const std::exception &e) {
            current_meta_data_.root_block_id = 0;
            current_meta_data_.newest_block_id = 0;
        }
    }
    return current_meta_data_;
}


Blockchain::MetaData Blockchain::getExternalMetaData(const std::string& folder_path) {
    MetaData meta_data;
    try {
        std::ifstream ifs(folder_path + "/meta");
        cereal::PortableBinaryInputArchive ia(ifs);
        ia >> meta_data;
    } catch (const std::exception &e) {
        meta_data.root_block_id = 0;
        meta_data.newest_block_id = 0;
    }
    return meta_data;
}


void Blockchain::setMetaData(MetaData& meta) {
    current_meta_data_ = meta;
    current_meta_data_initialized_ = true;
    std::ofstream ofs(folder_path_ + "/meta");
    cereal::PortableBinaryOutputArchive oa(ofs);
    oa << current_meta_data_;
}


void Blockchain::updateCurrentBaseline(const CollectionBlock& block) {
    LOCK_MUTEX_WATCHDOG(mtx_current_baseline_access_);
    current_baseline_.header.generic_header.block_hash = 0;
    current_baseline_.header.block_uid = getMetaData().newest_block_id + 1;
    current_baseline_.header.generic_header.previous_block_hash = block.header.generic_header.block_hash;

    assert(current_baseline_.data_value_hashes.size() == current_baseline_.mining_state.epoch+1);
    current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].reserve(CollectionBlock::max_num_creations);
    for(auto& creation : block.creations) {
        auto data_value_hash = CryptoHelper::calcHash(creation.second.data_value);
        current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].push_back(data_value_hash);
        current_baseline_.wallets[creation.second.creator] += TransactionSubBlock::fraction_per_coin;
    }
    std::sort(current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].begin(), current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].end());

    for(auto& transaction : block.transactions) {
        assert(current_baseline_.wallets[transaction.second.pre_owner] >= transaction.second.fraction);
        current_baseline_.wallets[transaction.second.pre_owner] -= transaction.second.fraction;
        current_baseline_.wallets[transaction.second.post_owner] += transaction.second.fraction;
    }

    current_baseline_.mining_state.highest_hash_of_current_epoch = current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].size() > 0 ?
            current_baseline_.data_value_hashes[current_baseline_.mining_state.epoch].back() : 0;
    current_baseline_.mining_state.num_minings_in_epoch += block.creations.size();
    if (current_baseline_.mining_state.num_minings_in_epoch == CollectionBlock::max_num_creations) {
        current_baseline_.mining_state.num_minings_in_epoch = 0;
        current_baseline_.mining_state.epoch++;
        current_baseline_.mining_state.highest_hash_of_last_epoch = current_baseline_.mining_state.highest_hash_of_current_epoch;
        current_baseline_.mining_state.highest_hash_of_current_epoch = 0;
        current_baseline_.data_value_hashes.resize(current_baseline_.mining_state.epoch + 1);
    }
}


std::ostream& scn::operator<<(std::ostream& os, const Blockchain& blockchain) {
    for(block_uid_t uid = blockchain.getRootBlockId();uid<=blockchain.getNewestBlockId();uid++) {
        auto baseblock = blockchain.getBlock(uid);
        switch(baseblock->header.generic_header.block_type) {
            case BlockType::BaselineBlock:
            default:
            {
                auto block = std::static_pointer_cast<scn::BaselineBlock>(baseblock);
                os << block << std::endl;
                break;
            }
            case BlockType::CollectionBlock:
            {
                auto block = std::static_pointer_cast<scn::CollectionBlock>(baseblock);
                os << block << std::endl;
                break;
            }
        }
    }
    return os;
}