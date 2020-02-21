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
#include <random>
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

}

MinerLocal::~MinerLocal() {
    stop();
}

void MinerLocal::start(const hash_t& previous_epoch_highest_hash,
                       const public_key_t& owner_public_key,
                       const epoch_t epoch,
                       std::function<void(epoch_t, const std::string &)> found_value_callback) {
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
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
#else
    struct sched_param p;
    p.sched_priority = 0;
    pthread_setschedparam(pthread_self(), SCHED_IDLE, &p);
#endif

    uint32_t random_value = 0;
    {
        std::default_random_engine generator(time(nullptr) + (thread_id * 12345));
        std::uniform_int_distribution<uint32_t> distribution(0, 1000000000);
        random_value = distribution(generator);
    }
    std::string random_prefix = std::to_string(random_value);
    std::string prefix = hash_helper::toString(previous_epoch_highest_hash_) + "_" +
            owner_public_key_.getAsShortString() + "_" + random_prefix + "_";
    CryptoHelper::Hash hash_object_common;
    hash_object_common.update(prefix);

    boost::multiprecision::uint1024_t value = 0;
    while(running_) {
        CryptoHelper::Hash hash_object_cycle = hash_object_common;
        auto payload = value.str(0, std::ios_base::hex | std::ios_base::uppercase);
        hash_object_cycle.update(payload);
        auto hash = hash_object_cycle.finalize();
        stats_check_counter_++;
        if (hash >= min_allowed_value_ && hash <= max_allowed_value_) {
            found_value_callback_(epoch_, prefix + payload);
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