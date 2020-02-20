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

#ifndef FULL_NODE_P2PCONNECTOR_H
#define FULL_NODE_P2PCONNECTOR_H

#include "IP2PConnector.h"
#include "scn/Blockchain/Blockchain.h"
#include <functional>
#include <vector>
#include <list>
#include <string>
#include <thread>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/session_status.hpp"
#include "libtorrent/peer_info.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/torrent_handle.hpp"
#include "libtorrent/create_torrent.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/extensions/LibTorrentFcoinPlugin.h"

namespace scn {

    class P2PConnector : public IP2PConnector, public libtorrent::IFcoinConnector {
    public:
        P2PConnector(uint16_t port, const Blockchain& blockchain);
        virtual ~P2PConnector();

        virtual void connect() override;

        virtual void disconnect() override;

        virtual uint32_t numConnectedPeers() const override;

        virtual void printPeerInfo() const;

        virtual void registerBlockCallbacks(std::function<void(const peer_id_t&, std::shared_ptr<const BaselineBlock>, bool)> callback_baseline,
                                            std::function<void(const peer_id_t&, std::shared_ptr<const CollectionBlock>, bool)> callback_collection) override;

        virtual void registerActivePeersCallback(std::function<void(const peer_id_t& peer_id, const ActivePeersList&)> callback_active_peers) override;

        virtual void askForBlock(block_uid_t uid) override;

        virtual void askForLastBaselineBlock() override;

        virtual void propagateBlock(const BaselineBlock& block) override;

        virtual void propagateBlock(const CollectionBlock& block) override;

        virtual void propagateActivePeersList(const ActivePeersList& active_peers_list) override;

        virtual void banPeer(const peer_id_t& peer_to_ban) override;

        virtual bool peerSendBuffersEmpty();

        virtual void registerPeer(libtorrent::IPeer& peer) override;

        virtual void unregisterPeer(libtorrent::IPeer& peer) override;

        virtual void receivedMessage(libtorrent::IPeer& peer, const std::string& compressed_message) override;

    protected:

        static const uint16_t protocol_version_;

        virtual void alertThread();

        std::list<libtorrent::IPeer*> getConnectedPeers() const;

        std::shared_ptr<libtorrent::session> session_;
        libtorrent::torrent_handle torrent_handle_;
        const Blockchain& blockchain_;

        bool running_;
        std::shared_ptr<std::thread> alert_thread_;

        std::atomic<libtorrent::IPeer*> peer_sending_baseline_to_;
        std::shared_ptr<std::thread> send_baseline_thread_;

        mutable std::mutex mtx_access_peers_;
        std::set<libtorrent::IPeer*> peers_;

        std::function<void(const peer_id_t&, std::shared_ptr<const BaselineBlock>, bool)> callback_baseline_;
        std::function<void(const peer_id_t&, std::shared_ptr<const CollectionBlock>, bool)> callback_collection_;
        std::function<void(const peer_id_t&, const ActivePeersList&)> callback_active_peers_;
    };

}

#endif //FULL_NODE_P2PCONNECTOR_H
