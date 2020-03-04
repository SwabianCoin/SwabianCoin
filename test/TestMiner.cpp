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

#include "scn/Miner/MinerLocal.h"
#include <gtest/gtest.h>

using namespace scn;


class TestMiner : public testing::Test {
public:

    TestMiner() {

    }

    void startMining(uint32_t num_worker_threads = 1) {
        miner_local = std::make_shared<MinerLocal>(num_worker_threads);
        miner_local->start(12345, example_owner_public_key, 10, std::bind(&TestMiner::foundHashCallback, this, std::placeholders::_1, std::placeholders::_2));
    }

    void stopMining() {
        if(miner_local) {
            miner_local->stop();
        }
    }

protected:

    void foundHashCallback(const epoch_t epoch, const std::string& data) {
        found_hashes_map_[data] = epoch;
    }

    std::shared_ptr<MinerLocal> miner_local;

    std::map<std::string, epoch_t> found_hashes_map_;

    public_key_t example_owner_public_key = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                            "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvY\n"
                                            "JzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                            "-----END PUBLIC KEY-----");
};


TEST_F(TestMiner, singleMiningThread) {
    startMining(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    stopMining();

    std::cout << "Number of Minings in 1000ms: " << found_hashes_map_.size() << std::endl;
    EXPECT_GT(found_hashes_map_.size(), 0);
}

TEST_F(TestMiner, multipleMiningThreads) {
    startMining(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    stopMining();

    std::cout << "Number of Minings in 1000ms: " << found_hashes_map_.size() << std::endl;
    EXPECT_GT(found_hashes_map_.size(), 0);
}

TEST_F(TestMiner, checksPerSecond) {
    startMining(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    auto num_checks_per_second = miner_local->numChecksPerSecond();
    stopMining();

    std::cout << "Number of checks per second: " << num_checks_per_second << std::endl;
    EXPECT_GT(num_checks_per_second, 0);
}

TEST_F(TestMiner, changeNumMiningThreads) {
    startMining(1);
    EXPECT_EQ(miner_local->numWorkerThreads(), 1);
    miner_local->changeNumWorkerThreads(3);
    EXPECT_EQ(miner_local->numWorkerThreads(), 3);
    stopMining();
}