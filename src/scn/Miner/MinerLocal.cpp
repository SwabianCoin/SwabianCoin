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

#include "MinerLocal.h"
#include "scn/CryptoHelper/CryptoHelper.h"
#include "scn/Blockchain/Blockchain.h"
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace scn;


MinerLocal::MinerLocal(uint32_t num_worker_threads)
:running_(false)
,num_worker_threads_(num_worker_threads)
,workers_()
,stats_check_counter_(0)
,stats_check_counter_per_sec_(0)
,previous_epoch_highest_hash_(0)
,epoch_(0) {
    std::srand(std::time(nullptr));
}

MinerLocal::~MinerLocal() {
    stop();
}

void MinerLocal::start(const hash_t& previous_epoch_highest_hash,
                       const public_key_t& owner_public_key,
                       const epoch_t epoch,
                       std::function<void(epoch_t, std::string &)> found_value_callback) {
    LOG(INFO) << "Mining epoch " << epoch;
    if(isRunning())
    {
        stop();
    }

    //init variables
    stats_check_counter_ = 0;
    stats_check_counter_per_sec_ = 0;
    previous_epoch_highest_hash_ = previous_epoch_highest_hash;
    owner_public_key_ = owner_public_key;
    epoch_ = epoch;
    found_value_callback_ = found_value_callback;
    running_ = true;

    //init hash areas
    Blockchain::getHashArea(epoch, max_allowed_value_, min_allowed_value_);

    //start worker threads
    workers_.reserve(num_worker_threads_);
    for(uint32_t i=0;i<num_worker_threads_;i++)
    {
        workers_.emplace_back(&MinerLocal::miningThread, this, i);

#ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
#else
        struct sched_param p;
        p.sched_priority=0;
        pthread_setschedparam(workers_.back().native_handle(),SCHED_IDLE,&p);
#endif
    }

    //start stats thread
    stats_thread_ = std::make_shared<std::thread>(&MinerLocal::statsThread, this);
}

void MinerLocal::stop(){
    running_ = false;

    if(stats_thread_ && stats_thread_->joinable()) {
        stats_thread_->join();
    }
    for(auto& worker : workers_)
    {
        try {
            worker.join();
        } catch(const std::system_error& e){

        }
    }
}

bool MinerLocal::isRunning() const{
    return running_;
}

uint32_t MinerLocal::numWorkerThreads() const{
    return num_worker_threads_;
}

void MinerLocal::changeNumWorkerThreads(uint32_t num_worker_threads) {
    stop();
    num_worker_threads_ = num_worker_threads;
    start(previous_epoch_highest_hash_, owner_public_key_, epoch_, found_value_callback_);
}

epoch_t MinerLocal::getEpoch() const{
    return epoch_;
}

uint64_t MinerLocal::numChecksPerSecond() const {
    return stats_check_counter_per_sec_;
}

void MinerLocal::miningThread(uint32_t thread_id)
{
    std::string random_prefix = std::to_string(std::rand() % 1000000000);
    std::string prefix = previous_epoch_highest_hash_.str(0, std::ios_base::hex) + "_" +
            owner_public_key_.getAsShortString() + "_" + random_prefix + "_";

    boost::multiprecision::uint1024_t value = 0;
    while(running_) {
        std::string string_to_hash = prefix + value.str(0, std::ios_base::hex);
        auto hash = CryptoHelper::calcHash(string_to_hash);
        stats_check_counter_++;
        if (hash >= min_allowed_value_ && hash <= max_allowed_value_) {
            found_value_callback_(epoch_, string_to_hash);
        }
        value++;
    }
}

void MinerLocal::statsThread()
{
    const uint32_t cycle_time_ms = 1000;
    auto next_cycle_time = std::chrono::system_clock::now();
    uint64_t last_stats_check_counter = stats_check_counter_;
    while(running_) {
        uint64_t current_stats_check_counter = stats_check_counter_;

        if(current_stats_check_counter < last_stats_check_counter) {
            stats_check_counter_per_sec_ = 0;
        } else {
            stats_check_counter_per_sec_ = (current_stats_check_counter - last_stats_check_counter) * 1000 / cycle_time_ms;
        }

        last_stats_check_counter = current_stats_check_counter;
        for(uint32_t i=0;i<10;i++) {
            next_cycle_time += std::chrono::milliseconds(cycle_time_ms/10);
            std::this_thread::sleep_until(next_cycle_time);
            if(!running_) {
                break;
            }
        }
    }
    stats_check_counter_per_sec_ = 0;
}