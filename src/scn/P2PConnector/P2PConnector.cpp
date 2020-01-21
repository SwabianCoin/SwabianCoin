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

#include "P2PConnector.h"
#include "P2PDefinitions.h"
#include "EntryPointFetcher.h"
#include <cereal/archives/portable_binary.hpp>
#include <boost/filesystem.hpp>
#include <functional>
#include <fstream>


using namespace scn;

const uint16_t P2PConnector::protocol_version_ = 1;


P2PConnector::P2PConnector(uint16_t port, const Blockchain& blockchain)
: blockchain_(blockchain)
, running_(true)
, peer_sending_baseline_to_(nullptr)
, callback_baseline_(nullptr)
, callback_collection_(nullptr)
, callback_active_peers_(nullptr) {
    LOG(INFO) << "Listen on port " << port;
    std::srand(std::time(nullptr));

    //create torrent
    {
        std::ofstream ofs("scn");
        ofs << "You are inside the swabian coin torrent network";
    }
    {
        libtorrent::file_storage fs;
        libtorrent::add_files(fs, "scn");
        libtorrent::create_torrent t(fs);
        t.set_creator("scn");
        for (auto &entry_point : EntryPointFetcher::fetch()) {
            t.add_node(std::pair<std::string, int>(entry_point.first, entry_point.second));
        }
        libtorrent::set_piece_hashes(t, ".");
        std::ofstream out("scn.torrent", std::ios_base::binary);
        libtorrent::bencode(std::ostream_iterator<char>(out), t.generate());
    }
    boost::filesystem::remove("scn");

    //initialize session
    {
        libtorrent::setFcoinConnector(this);
        libtorrent::settings_pack sett;
        sett.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:" + std::to_string(port));
        sett.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification |
                           lt::alert::peer_notification | /*lt::alert::debug_notification |*/
                           lt::alert::performance_warning | lt::alert::session_log_notification |
                           lt::alert::torrent_log_notification | /*lt::alert::peer_log_notification |*/
                           lt::alert::storage_notification | lt::alert::status_notification);
        sett.set_bool(libtorrent::settings_pack::enable_incoming_utp, false);
        sett.set_bool(libtorrent::settings_pack::enable_outgoing_utp, false);
        sett.set_bool(libtorrent::settings_pack::enable_lsd, false);
        session_ = std::make_shared<libtorrent::session>(sett);
        auto dht_settings = session_->get_dht_settings();
        session_->set_dht_settings(dht_settings);
        session_->add_extension(libtorrent::createFcoinPlugin);
        libtorrent::add_torrent_params p;
        p.save_path = "./";
        p.max_connections = 10;
        libtorrent::error_code ec;
        p.ti = std::make_shared<libtorrent::torrent_info>("scn.torrent", std::ref(ec));
        if (ec) {
            LOG(ERROR) << "error reading torrent file: " << ec.message();
            return;
        }
        torrent_handle_ = session_->add_torrent(p, ec);
        if (ec) {
            LOG(ERROR) << "error adding torrent file to session: " << ec.message();
            return;
        }
    }

    //start thread
    alert_thread_ = std::make_shared<std::thread>(&P2PConnector::alertThread, this);
}


P2PConnector::~P2PConnector() {
    libtorrent::setFcoinConnector(NULL);
    running_ = false;
    alert_thread_->join();
    if(send_baseline_thread_ && send_baseline_thread_->joinable()) {
        send_baseline_thread_->join();
    }
}


void P2PConnector::connect() {

}


void P2PConnector::disconnect() {
}


uint32_t P2PConnector::numConnectedPeers() const {
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    return peers_.size();
}


void P2PConnector::printPeerInfo() const {
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    for(auto& peer : peers_) {
        std::cout << peer->getInfo() << " - " << peer->getId() << std::endl;
    }
}


void P2PConnector::registerBlockCallbacks(std::function<void(IPeer&, const BaselineBlock&,bool)> callback_baseline,
                                    std::function<void(IPeer&, const CollectionBlock&,bool)> callback_collection) {
    callback_baseline_ = callback_baseline;
    callback_collection_ = callback_collection;
}


void P2PConnector::registerActivePeersCallback(std::function<void(IPeer&, const ActivePeersList&)> callback_active_peers) {
    callback_active_peers_ = callback_active_peers;
}


void P2PConnector::askForBlock(block_uid_t uid) {
    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    const MessageType type = MessageType::AskForBlock;
    oa << protocol_version_;
    oa << (uint8_t)type;
    oa << uid;
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    if(peers_.size() > 0) {
        auto it = peers_.begin();
        std::advance(it, std::rand() % peers_.size());
        if((*it) != peer_sending_baseline_to_) {
            (*it)->sendMessage(std::make_shared<std::string>(std::move(oss.str())));
        }
    }
}


void P2PConnector::askForLastBaselineBlock() {
    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    const MessageType type = MessageType::AskForLastBaselineBlock;
    oa << protocol_version_;
    oa << (uint8_t)type;
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    if(peers_.size() > 0) {
        auto it = peers_.begin();
        std::advance(it, std::rand() % peers_.size());
        if((*it) != peer_sending_baseline_to_) {
            (*it)->sendMessage(std::make_shared<std::string>(std::move(oss.str())));
        }
    }
}


void P2PConnector::propagateBlock(const BaselineBlock& block) {
    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    const MessageType type = MessageType::PropagateBaselineBlock;
    bool reply = false;
    oa << protocol_version_;
    oa << (uint8_t)type;
    oa << block;
    oa << reply;
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    auto output = std::make_shared<std::string>(std::move(oss.str()));
    for(auto& peer : peers_) {
        if(peer != peer_sending_baseline_to_) {
            peer->sendMessage(output);
        }
    }
}


void P2PConnector::propagateBlock(const CollectionBlock& block) {
    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    const MessageType type = MessageType::PropagateCollectionBlock;
    bool reply = false;
    oa << protocol_version_;
    oa << (uint8_t)type;
    oa << block;
    oa << reply;
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    auto output = std::make_shared<std::string>(std::move(oss.str()));
    for(auto& peer : peers_) {
        if(peer != peer_sending_baseline_to_) {
            peer->sendMessage(output);
        }
    }
}


void P2PConnector::propagateActivePeersList(const ActivePeersList& active_peers_list) {
    std::stringstream oss;
    cereal::PortableBinaryOutputArchive oa(oss);
    const MessageType type = MessageType::PropagateActivePeersList;
    oa << protocol_version_;
    oa << (uint8_t)type;
    oa << active_peers_list;
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    auto output = std::make_shared<std::string>(std::move(oss.str()));
    for(auto& peer : peers_) {
        if(peer != peer_sending_baseline_to_) {
            peer->sendMessage(output);
        }
    }
}


bool P2PConnector::peerSendBuffersEmpty() {
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    for(auto& peer : peers_) {
        if(!peer->sendBufferEmpty()) {
            return false;
        }
    }
    return true;
}


void P2PConnector::registerPeer(libtorrent::IPeer& peer) {
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    peers_.insert(&peer);
}


void P2PConnector::unregisterPeer(libtorrent::IPeer& peer) {
    LOCK_MUTEX_WATCHDOG(mtx_access_peers_);
    peers_.erase(&peer);
}


void P2PConnector::alertThread() {
    while (running_) {
        std::vector<lt::alert*> alerts;
        session_->pop_alerts(&alerts);

        if(alerts.size() > 0) {
            /*for (auto &alert : alerts) {
                if(alert->category() & lt::alert::error_notification) {
                    LOG(ERROR) << "Libtorrent alert: " << alert->category() << " " << alert->message();
                } else {
                    LOG(INFO) << "Libtorrent alert: " << alert->category() << " " << alert->message();
                }
            }*/
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}


void P2PConnector::receivedMessage(libtorrent::IPeer& peer, const std::string& message) {
    try {
        std::stringstream iss(message);
        cereal::PortableBinaryInputArchive ia(iss);
        uint16_t incoming_protocol_version;
        ia >> incoming_protocol_version;
        if(incoming_protocol_version != protocol_version_) {
            LOG(WARNING) << "Received unexpected protocol version - ours: " << protocol_version_ << " theirs: " << incoming_protocol_version;
            peer.ban();
            return;
        }
        MessageType type;
        ia >> type;
        switch (type) {
            case MessageType::PropagateBaselineBlock: {
                LOG(ERROR) << "PropagateBaselineBlock Start";
                if (callback_baseline_ != nullptr) {
                    BaselineBlock block;
                    bool reply;
                    ia >> block;
                    ia >> reply;
                    callback_baseline_(peer, block, reply);
                }
                LOG(ERROR) << "PropagateBaselineBlock End";
                break;
            }
            case MessageType::PropagateCollectionBlock: {
                if (callback_collection_ != nullptr) {
                    CollectionBlock block;
                    bool reply;
                    ia >> block;
                    ia >> reply;
                    callback_collection_(peer, block, reply);
                }
                break;
            }
            case MessageType::AskForLastBaselineBlock: {
                if(peer_sending_baseline_to_ == nullptr) {
                    peer_sending_baseline_to_ = &peer;
                    if(send_baseline_thread_ && send_baseline_thread_->joinable()) {
                        send_baseline_thread_->join();
                    }
                    send_baseline_thread_ = std::make_shared<std::thread>([&]() {
                        LOG(INFO) << "send_baseline_thread_ Start";
                        auto block = std::static_pointer_cast<scn::BaselineBlock>(blockchain_.getRootBlock());
                        if (block) {
                            std::stringstream oss;
                            cereal::PortableBinaryOutputArchive oa(oss);
                            const MessageType type = MessageType::PropagateBaselineBlock;
                            const bool reply = true;
                            oa << protocol_version_;
                            oa << (uint8_t)type;
                            oa << *block;
                            oa << reply;
                            if(peers_.find(&peer) != peers_.end()) {
                                peer.sendMessage(std::make_shared<std::string>(std::move(oss.str())));
                            }
                            while(peers_.find(&peer) != peers_.end() &&  !peer.sendBufferEmpty()) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            }
                        } else {
                            LOG(INFO) << "could not get AskForBaselineBlock answer";
                        }
                        peer_sending_baseline_to_ = nullptr;
                        LOG(INFO) << "send_baseline_thread_ End";
                    });
                }
                break;
            }
            case MessageType::AskForBlock: {
                if(&peer == peer_sending_baseline_to_) {
                    break;
                }

                block_uid_t uid;
                ia >> uid;
                auto baseblock = blockchain_.getBlock(uid);
                if (baseblock) {
                    switch (baseblock->header.generic_header.block_type) {
                        case BlockType::BaselineBlock:
                        default: {
                            auto block = std::static_pointer_cast<scn::BaselineBlock>(baseblock);
                            std::stringstream oss;
                            cereal::PortableBinaryOutputArchive oa(oss);
                            const MessageType type = MessageType::PropagateBaselineBlock;
                            const bool reply = true;
                            oa << protocol_version_;
                            oa << (uint8_t)type;
                            oa << *block;
                            oa << reply;
                            peer.sendMessage(std::make_shared<std::string>(std::move(oss.str())));
                            break;
                        }
                        case BlockType::CollectionBlock: {
                            auto block = std::static_pointer_cast<scn::CollectionBlock>(baseblock);
                            std::stringstream oss;
                            cereal::PortableBinaryOutputArchive oa(oss);
                            const MessageType type = MessageType::PropagateCollectionBlock;
                            const bool reply = true;
                            oa << protocol_version_;
                            oa << (uint8_t)type;
                            oa << *block;
                            oa << reply;
                            peer.sendMessage(std::make_shared<std::string>(std::move(oss.str())));
                            break;
                        }
                    }
                } else {
                    LOG(INFO) << "could not get AskForBlock answer";
                }
                break;
            }
            case MessageType::PropagateActivePeersList: {
                if (callback_active_peers_ != nullptr) {
                    ActivePeersList list;
                    ia >> list;
                    callback_active_peers_(peer, list);
                }
                break;
            }
            default: {
                LOG(ERROR) << "Unhandled incoming block type " << (uint32_t) type;
            }
        }
    } catch(std::exception& e) {
        LOG(ERROR) << "Error in received message: " << e.what() << " message: " << message;
    }
}
