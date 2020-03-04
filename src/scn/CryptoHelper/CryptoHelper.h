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

#ifndef FULL_NODE_CRYPTOHELPER_H
#define FULL_NODE_CRYPTOHELPER_H

#include "scn/Common/Common.h"
#include "scn/Blockchain/BlockDefinitions.h"
#include "HashStreamBuf.h"
#include <cereal/archives/portable_binary.hpp>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <sstream>


namespace scn {

    class CryptoHelper {
    public:
        CryptoHelper(const private_key_t& private_key);

        virtual ~CryptoHelper();

        static hash_t calcHash(const std::string& data);

        static hash_t calcHash(std::stringstream& data);

        static hash_t calcHash(const void* data, uint32_t length);

        template<class BLOCK>
        static void fillHash(BLOCK& block);

        template<class BLOCK>
        static bool verifyHash(const BLOCK& block);

        virtual signature_t calcSignature(const std::string& data);

        static bool verifySignature(const std::string& data, const signature_t& signature, const public_key_t& public_key);

        template<class BLOCK>
        void fillSignature(BLOCK& block);

        template<class BLOCK>
        static bool verifySignature(const BLOCK& block, const public_key_t& public_key);

        static bool isPublicKeyValid(const public_key_t& public_key);

        static bool isPrivateKeyValid(const private_key_t& private_key);

        class Hash {
        public:
            Hash();
            void update(const std::string& data);
            hash_t finalize();
        private:
            SHA256_CTX context_;
        };

    private:
        static EC_KEY* createPublicEC(const public_key_t& public_key);
        static EC_KEY* createPrivateEC(const private_key_t& private_key);

        static size_t calcDecodeLength(const std::string &val);
        static std::string decode64(const std::string &val);
        static std::string encode64(const std::string &val);

        EC_KEY* private_ec_;
    };

    template<class BLOCK>
    void CryptoHelper::fillHash(BLOCK& block) {
        assert(block.header.generic_header.block_hash == 0);
        HashStreamBuf hash_buf;
        std::ostream hash_stream(&hash_buf);
        cereal::PortableBinaryOutputArchive oa(hash_stream);
        oa << block;
        block.header.generic_header.block_hash = hash_buf.digestHash();
    }

    //todo: avoid copying block
    template<class BLOCK>
    bool CryptoHelper::verifyHash(const BLOCK& block) {
        BLOCK block_copy = block;
        block_copy.header.generic_header.block_hash = 0;
        HashStreamBuf hash_buf;
        std::ostream hash_stream(&hash_buf);
        cereal::PortableBinaryOutputArchive oa(hash_stream);
        oa << block_copy;
        return block.header.generic_header.block_hash == hash_buf.digestHash();
    }

    template<class BLOCK>
    void CryptoHelper::fillSignature(BLOCK& block) {
        assert(block.header.generic_header.block_hash == 0);
        assert(block.signature == "");
        std::stringstream oss;
        cereal::PortableBinaryOutputArchive oa(oss);
        oa << block;
        block.signature = calcSignature(oss.str());
    }

    template<class BLOCK>
    bool CryptoHelper::verifySignature(const BLOCK& block, const public_key_t& public_key) {
        BLOCK block_copy = block;
        block_copy.header.generic_header.block_hash = 0;
        block_copy.signature = "";
        std::stringstream oss;
        cereal::PortableBinaryOutputArchive oa(oss);
        oa << block_copy;
        return verifySignature(oss.str(), block.signature, public_key);
    }
}

#endif //FULL_NODE_CRYPTOHELPER_H
