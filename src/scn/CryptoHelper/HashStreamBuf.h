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

#ifndef FULL_NODE_HASHSTREAMBUF_H
#define FULL_NODE_HASHSTREAMBUF_H

#include <streambuf>
#include <memory>
#include "scn/Common/Common.h"
#include <openssl/sha.h>

namespace scn {

    class HashStreamBuf : public std::streambuf {
    public:
        HashStreamBuf(uint32_t buffer_size = 8192);

        virtual ~HashStreamBuf();

        hash_t digestHash();

    protected:

        bool flushBuffer();

        virtual int_type overflow (int_type c) override;

        virtual int sync() override;

        std::vector<char> buffer_;
        SHA256_CTX sha256_;
    };

}


#endif //FULL_NODE_HASHSTREAMBUF_H
