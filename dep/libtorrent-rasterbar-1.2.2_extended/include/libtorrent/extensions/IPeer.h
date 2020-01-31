
#ifndef LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IPEER_H
#define LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IPEER_H

#include <string>
#include <memory>

namespace libtorrent {

    class IPeer {
    public:
        IPeer() {};
        virtual ~IPeer() {};

        virtual void sendMessage(std::shared_ptr<std::string> message) = 0;

        virtual std::string getInfo() const = 0;

        virtual bool sendBufferEmpty() const = 0;

        virtual void kick() = 0;

        virtual void ban() = 0;

        virtual std::string getId() const = 0;

        virtual bool isConnected() const = 0;
    };

}

#endif //LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IPEER_H
