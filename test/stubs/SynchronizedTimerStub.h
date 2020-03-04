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

#ifndef FULL_NODE_SYNCHRONIZEDTIMERSTUB_H
#define FULL_NODE_SYNCHRONIZEDTIMERSTUB_H

#include "scn/SynchronizedTime/ISynchronizedTimer.h"

namespace scn {

    class SynchronizedTimerStub : public ISynchronizedTimer {
    public:
        SynchronizedTimerStub(blockchain_time_t initial_time = 12345)
        :current_time_(initial_time) {};

        virtual ~SynchronizedTimerStub() {};

        virtual blockchain_time_t now() const {
            return current_time_;
        }

        virtual void letTheTimeGoOn(uint32_t duration_ms) {
            current_time_ += duration_ms;
        }

        virtual void letTheTimeGoBack(uint32_t duration_ms) {
            current_time_ -= duration_ms;
        }

        virtual void resetTime(blockchain_time_t new_time) {
            current_time_ = new_time;
        }

    protected:
        blockchain_time_t current_time_;
    };

}

#endif //FULL_NODE_SYNCHRONIZEDTIMERSTUB_H
