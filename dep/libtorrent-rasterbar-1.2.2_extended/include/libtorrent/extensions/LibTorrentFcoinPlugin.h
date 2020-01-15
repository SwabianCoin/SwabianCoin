
#ifndef LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_LIBTORRENTFCOINPLUGIN_H
#define LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_LIBTORRENTFCOINPLUGIN_H

#include "IFcoinConnector.h"
#include <libtorrent/aux_/export.hpp>
#include <boost/shared_ptr.hpp>

namespace libtorrent {

    struct torrent_plugin;
    struct torrent_handle;

    TORRENT_EXPORT std::shared_ptr<libtorrent::torrent_plugin> createFcoinPlugin(libtorrent::torrent_handle const&, void*);

    TORRENT_EXPORT void setFcoinConnector(IFcoinConnector* fcoin_connector);
}

#endif //LIBTORRENT_RASTERBAR_1_1_13_EXTENDED_LIBTORRENTFCOINPLUGIN_H
