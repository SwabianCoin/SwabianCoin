
#include <libtorrent/extensions/LibTorrentFcoinPlugin.h>

#include <libtorrent/torrent.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/extensions.hpp>
#include <libtorrent/peer_connection.hpp>
#include <libtorrent/bt_peer_connection.hpp>
#include <libtorrent/peer_connection_handle.hpp>
#include <libtorrent/bencode.hpp>

#include <queue>
#include <thread>
#include <libtorrent/peer_info.hpp>

using namespace libtorrent;

static IFcoinConnector *fcoin_connector_ = NULL;

void libtorrent::setFcoinConnector(IFcoinConnector *fcoin_connector) {
    fcoin_connector_ = fcoin_connector;
}

struct FcoinPeerPlugin;

struct FcoinPlugin final : torrent_plugin
{
    FcoinPlugin(torrent& t)
    :torrent_(t) {

    }

    virtual std::shared_ptr<peer_plugin> new_connection(peer_connection_handle const& pc) override;

protected:
    torrent& torrent_;
};

struct FcoinPeerPlugin final
        : peer_plugin, std::enable_shared_from_this<FcoinPeerPlugin>, IPeer
{
    friend struct FcoinPlugin;

    FcoinPeerPlugin(torrent& t, bt_peer_connection& pc, FcoinPlugin& tp)
    : libtorrent_thread_id_(std::this_thread::get_id())
    , torrent_(t)
    , peer_connection_(pc) {
        (void)tp;
        if(!peer_connection_.associated_torrent().expired()) {
            std::lock_guard<std::mutex> lock(mtx_peer_info_access_);
            peer_connection_.get_peer_info(peer_info_);
        }
        if(fcoin_connector_ != NULL) {
            fcoin_connector_->registerPeer(*this);
        }
    }

    virtual ~FcoinPeerPlugin() {
        if(fcoin_connector_ != NULL) {
            fcoin_connector_->unregisterPeer(*this);
        }
    }

    virtual string_view type() const override { return "fcoin_plugin"; }

    virtual void add_handshake(entry& h) override
    {
        entry& messages = h["m"];
        messages["fcoin_plugin"] = 117;
    }

    virtual bool on_extension_handshake(bdecode_node const& h) override
    {
        if (h.type() != bdecode_node::dict_t) return false;
        bdecode_node messages = h.dict_find_dict("m");
        if (!messages) return false;

        int index = messages.dict_find_int_value("fcoin_plugin", -1);
        if (index == -1) return false;

        return true;
    }

    virtual bool on_extended(int length, int extended_msg, span<char const> body) override {
        //todo: avoid copying
        if(extended_msg == 117) {
            if (fcoin_connector_ != NULL) {
                if(body.size() < length) {
                    return true;
                }
                fcoin_connector_->receivedMessage(*this, std::string(body.begin(), body.size()));
            }
        }
        return true;
    }

    virtual bool on_piece(peer_request const& piece, span<char const> buf) override
    {
        (void)piece;
        (void)buf;
        return true; //make sure that no pieces are ever received to keep connections alive
    }

    virtual void tick() override {
        if(!peer_connection_.associated_torrent().expired()) {
            std::lock_guard<std::mutex> lock(mtx_peer_info_access_);
            peer_connection_.get_peer_info(peer_info_);
        }
        {
            std::lock_guard<std::mutex> lock(mtx_buffer_access_);
            while(!buffered_msgs_.empty()) {
                sendMessageWithinSingleThread(buffered_msgs_.front());
                buffered_msgs_.pop();
            }
        }
    }

    virtual void sendMessage(std::shared_ptr<std::string> message) {
        if(libtorrent_thread_id_ == std::this_thread::get_id()) {
            sendMessageWithinSingleThread(message);
        } else {
            std::lock_guard<std::mutex> lock(mtx_buffer_access_);
            buffered_msgs_.push(message);
        }
    }

    virtual std::string getInfo() const {
        std::lock_guard<std::mutex> lock(mtx_peer_info_access_);
        return peer_info_.ip.address().to_string() + ":" + std::to_string(peer_info_.ip.port());
    }

    virtual bool sendBufferEmpty() const {
        std::lock_guard<std::mutex> lock(mtx_buffer_access_);
        return peer_connection_.m_send_buffer.size() == 0 && buffered_msgs_.size() == 0;
    }

    virtual void kick() {
        peer_connection_.disconnect(errors::optimistic_disconnect, operation_t::bittorrent, peer_connection_interface::peer_error);
    }

    virtual void ban() {
        torrent_.ban_peer(peer_connection_.peer_info_struct());
        peer_connection_.disconnect(errors::peer_banned, operation_t::bittorrent, peer_connection_interface::peer_error);
    }

    virtual std::string getId() const {
        return peer_connection_.pid().to_string();
    }

protected:

    virtual void sendMessageWithinSingleThread(std::shared_ptr<std::string> message) {
        std::vector<char> header_buffer(6);
        char* header = &header_buffer[0];
        int total_size = 2 + message->length();
        detail::write_uint32(total_size, header);
        detail::write_uint8(bt_peer_connection::msg_extended, header);
        detail::write_uint8(117, header);
        peer_connection_.send_buffer({&header_buffer[0], 6});
        peer_connection_.send_buffer({&message->c_str()[0], static_cast<libtorrent::span<const char>::difference_type>(message->length())});
    }

    std::thread::id libtorrent_thread_id_;

    torrent& torrent_;
    bt_peer_connection& peer_connection_;

    mutable std::mutex mtx_peer_info_access_;
    peer_info peer_info_;

    mutable std::mutex mtx_buffer_access_;
    std::queue<std::shared_ptr<std::string>> buffered_msgs_;
};


std::shared_ptr<peer_plugin> FcoinPlugin::new_connection(peer_connection_handle const& pc)
{
    if (pc.type() != connection_type::bittorrent)
        return std::shared_ptr<peer_plugin>();

    bt_peer_connection* c = static_cast<bt_peer_connection*>(pc.native_handle().get());
    return std::shared_ptr<peer_plugin>(new FcoinPeerPlugin(torrent_, *c, *this));
}


std::shared_ptr<torrent_plugin> libtorrent::createFcoinPlugin(torrent_handle const& th, void*) {
    torrent *t = th.native_handle().get();
    // don't add this extension if the torrent is private
    if (t->valid_metadata() && t->torrent_file().priv()) return std::shared_ptr<torrent_plugin>();
    return std::shared_ptr<torrent_plugin>(new FcoinPlugin(*t));
}