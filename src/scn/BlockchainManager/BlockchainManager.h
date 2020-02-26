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

#ifndef FULL_NODE_BLOCKCHAINMANAGER_H
#define FULL_NODE_BLOCKCHAINMANAGER_H

#include "scn/Common/Common.h"
#include "scn/Blockchain/Blockchain.h"
#include "scn/Blockchain/BlockDefinitions.h"
#include "scn/SynchronizedTime/ISynchronizedTimer.h"
#include "scn/SynchronizedTime/SynchronizedTimer.h"
#include "scn/P2PConnector/IP2PConnector.h"
#include "scn/Miner/IMiner.h"
#include "scn/BlockchainManager/OutOfSyncDetector.h"
#include "scn/BlockchainManager/PeersMonitor.h"
#include "scn/BlockchainManager/ActivePeersCollector.h"
#include "scn/BlockchainManager/ICycleState.h"
#include "scn/BlockchainManager/CycleStateFetchBlockchain.h"
#include "scn/BlockchainManager/CycleStateCollect.h"
#include "scn/BlockchainManager/CycleStateIntroduceBlock.h"
#include "scn/BlockchainManager/CycleStateIntroduceBaseline.h"
#include "scn/CryptoHelper/CryptoHelper.h"
#include <mutex>
#include <thread>
#include <map>
#include <queue>

namespace scn {

    class BlockchainManager {
    public:
        BlockchainManager(const public_key_t& our_public_key,
                const private_key_t& our_private_key,
                Blockchain& blockchain,
                IP2PConnector& p2p_connector,
                IMiner& miner,
                bool initial_fetch = true,
                ISynchronizedTimer& sync_timer = static_sync_timer);
        virtual ~BlockchainManager();

        virtual void baselineBlockReceivedCallback(const peer_id_t& peer_id, const std::shared_ptr<const BaselineBlock>& block, bool reply);

        virtual void collectionBlockReceivedCallback(const peer_id_t& peer_id, std::shared_ptr<const CollectionBlock> block, bool reply);

        virtual void foundHashCallback(epoch_t epoch, const std::string& data);

        virtual void triggerTransaction(const public_key_t& receiver, uint64_t fraction);

        virtual void join();

        virtual uint8_t percentBlockchainSynchronized() const;

        virtual uint64_t getTotalPeersEstimation() const;

        virtual ICycleState::State getCurrentState() const;

        static bool isBaselineBlock(block_uid_t block_uid);

        static block_uid_t getNextBaselineBlock(block_uid_t block_uid);

        static block_uid_t getPreviousBaselineBlock(block_uid_t block_uid);

        static block_uid_t getBlockId(blockchain_time_t blockchain_time);

    protected:

        static SynchronizedTimer static_sync_timer;

        static const uint32_t max_num_blocks_after_baseline_ = 720;

        virtual void pauseMiner();

        virtual void resumeMiner();

        virtual uint32_t setState(ICycleState& new_state);

        virtual bool cycleUntil(blockchain_time_t target_time);

        virtual void updateStateThread();

        void fetchBlockchain();

        static const blockchain_time_t cycle_length_ms_ = 120000;

        public_key_t our_public_key_;
        private_key_t our_private_key_;

        ISynchronizedTimer& sync_timer_;
        CryptoHelper crypto_;
        Blockchain& blockchain_;
        IP2PConnector& p2p_connector_;
        IMiner& miner_;
        std::mutex mtx_control_miner_;
        bool initial_fetch_;

        OutOfSyncDetector out_of_sync_detector_;
        PeersMonitor peers_monitor_;
        ActivePeersCollector active_peers_collector_;

        CycleStateFetchBlockchain cycle_state_fetch_blockchain_;
        CycleStateCollect cycle_state_collect_;
        CycleStateIntroduceBlock cycle_state_introduce_block_;
        CycleStateIntroduceBaseline cycle_state_introduce_baseline_;

        friend class CycleStateFetchBlockchain;
        friend class CycleStateCollect;
        friend class CycleStateIntroduceBlock;
        friend class CycleStateIntroduceBaseline;

        mutable std::recursive_mutex mtx_current_state_access_;
        ICycleState* current_state_;

        bool running_;
        std::unique_ptr<std::thread> update_state_thread_;

        CollectionBlock new_block_;

        std::mutex mtx_found_hash_queue_access_;
        std::queue<std::string> found_hash_queue_;

        std::mutex mtx_transaction_queue_access_;
        std::queue<std::pair<const public_key_t,uint64_t>> transaction_queue_;
    };

}

#endif //FULL_NODE_BLOCKCHAINMANAGER_H
