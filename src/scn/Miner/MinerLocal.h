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

#ifndef FULL_NODE_MINERLOCAL_H
#define FULL_NODE_MINERLOCAL_H

#include "IMiner.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <boost/multiprecision/cpp_int.hpp>

namespace scn {

    class MinerLocal : public IMiner {
    public:
        MinerLocal(uint32_t num_worker_threads);
        virtual ~MinerLocal();

        virtual void start(const hash_t& previous_epoch_highest_hash,
                           const public_key_t& owner_public_key,
                           const epoch_t epoch,
                           std::function<void(epoch_t,std::string&)> found_value_callback) override;

        virtual void stop() override;

        virtual bool isRunning() const override;

        virtual uint32_t numWorkerThreads() const;

        virtual void changeNumWorkerThreads(uint32_t num_worker_threads);

        virtual epoch_t getEpoch() const override;

        virtual uint64_t numChecksPerSecond() const override;

    protected:

        virtual void miningThread(uint32_t thread_id);

        virtual void statsThread();

        std::atomic<bool> running_;

        uint32_t num_worker_threads_;
        std::vector<std::thread> workers_;

        std::atomic<uint64_t> stats_check_counter_;
        std::atomic<uint64_t> stats_check_counter_per_sec_;
        std::shared_ptr<std::thread> stats_thread_;

        hash_t previous_epoch_highest_hash_;
        public_key_t owner_public_key_;
        epoch_t epoch_;
        std::function<void(epoch_t,std::string&)> found_value_callback_;
        hash_t min_allowed_value_;
        hash_t max_allowed_value_;
    };

}

#endif //FULL_NODE_MINERLOCAL_H
