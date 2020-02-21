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

#include "SynchronizedTimer.h"
#include <chrono>

using namespace scn;


SynchronizedTimer::SynchronizedTimer() = default;


SynchronizedTimer::~SynchronizedTimer() = default;


blockchain_time_t SynchronizedTimer::now() const {
    //todo: synchronize with ntp server - at the moment we trust here that the system clock is synchronized automatically, which is the case in ubuntu 18.04
    return std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
}