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

#ifndef FULL_NODE_PEERSTUB_H
#define FULL_NODE_PEERSTUB_H

#include "libtorrent/extensions/IPeer.h"

namespace scn {

    class PeerStub : public IPeer {
    public:
        PeerStub() {}

        virtual ~PeerStub() {}

        virtual void sendMessage(std::shared_ptr<std::string> message) {

        }

        virtual std::string getInfo() const {

        }

        virtual bool sendBufferEmpty() const {

        }

        virtual void kick() {

        }

        virtual void ban() {

        }

        virtual std::string getId() const {

        }

        virtual bool isConnected() const {

        }
    };

}

#endif //FULL_NODE_PEERSTUB_H
