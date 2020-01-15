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

#ifndef FULL_NODE_SYSTEMMONITOR_H
#define FULL_NODE_SYSTEMMONITOR_H

#include <scn/Common/Common.h>
#include <thread>

namespace scn {

    class SystemMonitor {
    public:
        SystemMonitor();
        virtual ~SystemMonitor();

    protected:

        void systemMonitorThread();

        bool running_;
        std::thread system_monitor_thread_;
    };

}

#endif //FULL_NODE_SYSTEMMONITOR_H
