
#ifndef LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IFCOINCONNECTOR_H
#define LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IFCOINCONNECTOR_H

#include <string>
#include "IPeer.h"

namespace libtorrent {

    class IFcoinConnector {
    public:
        IFcoinConnector() {};
        virtual ~IFcoinConnector() {};

        virtual void registerPeer(IPeer& peer) = 0;

        virtual void unregisterPeer(IPeer& peer) = 0;

        virtual void receivedMessage(IPeer& peer, const std::string& message) = 0;
    };

}

#endif //LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_IFCOINCONNECTOR_H
