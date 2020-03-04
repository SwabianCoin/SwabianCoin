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

#include "stubs/SynchronizedTimerStub.h"
#include "stubs/P2PConnectorStub.h"
#include "scn/BlockchainManager/PeersMonitor.h"
#include <gtest/gtest.h>

using namespace scn;


class TestPeersMonitor : public testing::Test {
public:

    TestPeersMonitor()
    :sync_timer_()
    ,p2p_connector_()
    ,peers_monitor_(sync_timer_, p2p_connector_) {

    }

protected:

    SynchronizedTimerStub sync_timer_;
    P2PConnectorStub p2p_connector_;
    PeersMonitor  peers_monitor_;

};


TEST_F(TestPeersMonitor, ValidPeer) {
    CollectionBlock block;
    for(auto i=0;i<500;i++) {
        peers_monitor_.blockReceivedCallback("PEER_ID_0", block, false);
        sync_timer_.letTheTimeGoOn(4000);
    }
    EXPECT_EQ(p2p_connector_.ban_peer_counter_, 0);
}

TEST_F(TestPeersMonitor, PeerSendingTooFast) {
    CollectionBlock block;
    for(auto i=0;i<500;i++) {
        peers_monitor_.blockReceivedCallback("PEER_ID_0", block, false);
        sync_timer_.letTheTimeGoOn(2000);
    }
    ASSERT_GE(p2p_connector_.ban_peer_counter_, 1);
    EXPECT_EQ(p2p_connector_.last_banned_peer_id_, "PEER_ID_0");
}

TEST_F(TestPeersMonitor, ReportViolation) {
    for(auto i=0;i<100;i++) {
        peers_monitor_.reportViolation("PEER_ID_0");
    }
    ASSERT_GE(p2p_connector_.ban_peer_counter_, 1);
    EXPECT_EQ(p2p_connector_.last_banned_peer_id_, "PEER_ID_0");
}

