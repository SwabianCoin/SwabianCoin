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

#ifndef FULL_NODE_ISYNCHRONIZEDTIMER_H
#define FULL_NODE_ISYNCHRONIZEDTIMER_H

#include <scn/Common/Common.h>

namespace scn {

    class ISynchronizedTimer {
    public:
        ISynchronizedTimer() {};
        virtual ~ISynchronizedTimer() {};

        virtual blockchain_time_t now() const = 0;
    };

}

#endif //FULL_NODE_ISYNCHRONIZEDTIMER_H
