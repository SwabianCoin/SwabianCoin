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

#ifndef FULL_NODE_IMINER_H
#define FULL_NODE_IMINER_H

#include "scn/Common/Common.h"
#include "scn/Blockchain/BlockDefinitions.h"
#include <functional>
#include <string>

namespace scn {

    class IMiner {
    public:
        IMiner() {};
        virtual ~IMiner() {};

        virtual void start(const hash_t& previous_epoch_highest_hash,
                const public_key_t& owner_public_key,
                const epoch_t epoch,
                std::function<void(epoch_t,std::string&)> found_value_callback) = 0;

        virtual void stop() = 0;

        virtual bool isRunning() const = 0;

        virtual epoch_t getEpoch() const = 0;

        virtual uint64_t numChecksPerSecond() const = 0;
    };

}

#endif //FULL_NODE_IMINER_H
