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

#include "HashStreamBuf.h"

using namespace scn;


HashStreamBuf::HashStreamBuf(uint32_t buffer_size)
:buffer_(buffer_size + 1) {
    char* base = &buffer_.front();
    setp (base, base + buffer_.size() - 1);

    SHA256_Init(&sha256_);
}


HashStreamBuf::~HashStreamBuf() {
    sync();
}


hash_t HashStreamBuf::digestHash() {
    (void)flushBuffer();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256_);
    boost::multiprecision::uint256_t ret;
    for (int i = 0; i < 32; i++) {
        ret += boost::multiprecision::uint256_t(hash[i]) * pow(boost::multiprecision::uint256_t(2), 8 * (32 - i - 1));
    }
    SHA256_Init(&sha256_);
    return ret;
}


bool HashStreamBuf::flushBuffer() {
    int num = pptr() - pbase();
    SHA256_Update(&sha256_, &buffer_.front(), num);
    pbump (-num);
    return num;
}


std::streambuf::int_type HashStreamBuf::overflow (int_type c) {
    if (c != traits_type::eof()) {
        *pptr() = c;
        pbump(1);
        if (flushBuffer()) {
            return c;
        }
    }
    return traits_type::eof();
}


int HashStreamBuf::sync() {
    return flushBuffer() ? 0 : -1;
}
