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

#ifndef FULL_NODE_COMMON_H
#define FULL_NODE_COMMON_H

#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <glog/logging.h>
#include <chrono>
#include "PublicKeyPEM.h"

namespace scn {

    typedef boost::multiprecision::uint256_t hash_t;

    typedef PublicKeyPEM public_key_t;

    typedef std::string private_key_t;

    typedef std::string signature_t;

    typedef uint64_t epoch_t;

    typedef uint64_t block_uid_t; //0 = invalid

    typedef uint64_t blockchain_time_t;

    typedef std::string peer_id_t;

    #define LOCK_MUTEX_WATCHDOG(mtx) std::chrono::time_point<std::chrono::system_clock> t_start, t_stop; \
                                     t_start = std::chrono::system_clock::now(); \
                                     std::lock_guard<std::mutex> lock(mtx); \
                                     t_stop = std::chrono::system_clock::now(); \
                                     auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(t_stop - t_start).count(); \
                                     if(millis >= 150) LOG(WARNING) << "Mutex watchdog: locking takes long time - " << millis << " ms";

#define LOCK_MUTEX_WATCHDOG_REC(mtx) std::chrono::time_point<std::chrono::system_clock> t_start, t_stop; \
                                     t_start = std::chrono::system_clock::now(); \
                                     std::lock_guard<std::recursive_mutex> lock(mtx); \
                                     t_stop = std::chrono::system_clock::now(); \
                                     auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(t_stop - t_start).count(); \
                                     if(millis >= 150) LOG(WARNING) << "Mutex watchdog: locking takes long time - " << millis << " ms";

    namespace hash_helper {

        std::string toString(const hash_t& hash);

        void toArray(const hash_t& hash, uint8_t* array);

        hash_t fromArray(const uint8_t* array);

    }
}


#endif //FULL_NODE_COMMON_H
