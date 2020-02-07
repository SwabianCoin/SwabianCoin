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

#include "CryptoHelper.h"
#include <openssl/sha.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>


using namespace scn;

CryptoHelper::CryptoHelper(const public_key_t& public_key, const private_key_t& private_key) {
    public_ec_ = createPublicEC(public_key);
    private_ec_ = createPrivateEC(private_key);
    if(public_ec_ == NULL) {
        LOG(ERROR) << "Public key invalid!";
    } else if(private_ec_ == NULL) {
        LOG(ERROR) << "Private key invalid!";
    }
}

CryptoHelper::~CryptoHelper() {
    EC_KEY_free(public_ec_);
    EC_KEY_free(private_ec_);
}

hash_t CryptoHelper::calcHash(const std::string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.length());
    SHA256_Final(hash, &sha256);
    return hash_helper::fromArray(hash);
}

hash_t CryptoHelper::calcHash(std::stringstream& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    char buffer[1024*1024];
    while(!data.eof()) {
        data.read(buffer, sizeof(buffer));
        auto bytes_read = data.gcount();
        SHA256_Update(&sha256, buffer, bytes_read);
    }
    SHA256_Final(hash, &sha256);
    return hash_helper::fromArray(hash);
}

hash_t CryptoHelper::calcHash(const void* data, uint32_t length) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(hash, &sha256);
    return hash_helper::fromArray(hash);
}

signature_t CryptoHelper::calcSignature(const std::string &data) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_PKEY *private_key = EVP_PKEY_new();
    EVP_PKEY_assign_EC_KEY(private_key, private_ec_);
    if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, private_key) <= 0) {
        EVP_MD_CTX_destroy(ctx);
        return signature_t();
    }
    if (EVP_DigestSignUpdate(ctx, data.c_str(), data.length()) <= 0) {
        EVP_MD_CTX_destroy(ctx);
        return signature_t();
    }
    size_t msg_len_enc;
    if (EVP_DigestSignFinal(ctx, NULL, &msg_len_enc) <= 0) {
        EVP_MD_CTX_destroy(ctx);
        return signature_t();
    }
    unsigned char *enc_msg = (unsigned char *) malloc(msg_len_enc);
    if (EVP_DigestSignFinal(ctx, enc_msg, &msg_len_enc) <= 0) {
        free(enc_msg);
        EVP_MD_CTX_destroy(ctx);
        return signature_t();
    }
    EVP_MD_CTX_destroy(ctx);

    std::string ret = encode64(std::string(reinterpret_cast<char*>(enc_msg), msg_len_enc));

    free(enc_msg);
    return ret;
}

bool CryptoHelper::verifySignature(const std::string &data, const signature_t &signature, const public_key_t& public_key) {
    std::string signature_raw = decode64(signature);
    EC_KEY* public_ec = createPublicEC(public_key);
    if(public_ec == NULL) {
        return false;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_PKEY *public_key_int = EVP_PKEY_new();
    EVP_PKEY_assign_EC_KEY(public_key_int, public_ec);
    if (EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, public_key_int) <= 0) {
        EC_KEY_free(public_ec);
        EVP_MD_CTX_destroy(ctx);
        return false;
    }
    if (EVP_DigestVerifyUpdate(ctx, data.c_str(), data.length()) <= 0) {
        EC_KEY_free(public_ec);
        EVP_MD_CTX_destroy(ctx);
        return false;
    }
    int AuthStatus = EVP_DigestVerifyFinal(ctx, reinterpret_cast<const unsigned char*>(signature_raw.c_str()), signature_raw.length());
    if (AuthStatus == 1) {
        EC_KEY_free(public_ec);
        EVP_MD_CTX_destroy(ctx);
        return true;
    } else if (AuthStatus == 0) {
        EC_KEY_free(public_ec);
        EVP_MD_CTX_destroy(ctx);
        return false;
    } else {
        EC_KEY_free(public_ec);
        EVP_MD_CTX_destroy(ctx);
        return false;
    }
}


bool CryptoHelper::isPublicKeyValid(const public_key_t& public_key) {
    EC_KEY* key = createPublicEC(public_key);
    bool valid = (key != NULL);
    EC_KEY_free(key);
    return valid;
}


bool CryptoHelper::isPrivateKeyValid(const private_key_t& private_key) {
    EC_KEY* key = createPrivateEC(private_key);
    bool valid = (key != NULL);
    EC_KEY_free(key);
    return valid;
}


EC_KEY *CryptoHelper::createPublicEC(const public_key_t &public_key) {
    EC_KEY *ec_key = NULL;
    BIO *keybio;
    auto public_key_string = public_key.getAsFullString();
    keybio = BIO_new_mem_buf((void *)public_key_string.c_str(), public_key_string.length());
    if (keybio == NULL) {
        return NULL;
    }
    ec_key = PEM_read_bio_EC_PUBKEY(keybio, &ec_key, NULL, NULL);
    BIO_free(keybio);
    return ec_key;
}


EC_KEY *CryptoHelper::createPrivateEC(const private_key_t &private_key) {
    EC_KEY *ec_key = NULL;
    BIO *keybio = BIO_new_mem_buf((void *) private_key.c_str(), private_key.length());
    if (keybio == NULL) {
        return NULL;
    }
    ec_key = PEM_read_bio_ECPrivateKey(keybio, &ec_key, NULL, NULL);
    BIO_free(keybio);
    return ec_key;
}

size_t CryptoHelper::calcDecodeLength(const std::string &val) {
    size_t len = val.length(), padding = 0;

    if (val[len-1] == '=' && val[len-2] == '=') //last two chars are =
        padding = 2;
    else if (val[len-1] == '=') //last char is =
        padding = 1;

    return (len*3)/4 - padding;
}

std::string CryptoHelper::decode64(const std::string &val) {
    try {
        using namespace boost::archive::iterators;
        auto signature_raw_size = calcDecodeLength(val);
        using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
        return std::string(It(std::begin(val)), It(std::end(val))).substr(0, signature_raw_size);
    } catch(const std::exception& e) {
        return "";
    }
}

std::string CryptoHelper::encode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

CryptoHelper::Hash::Hash() {
    SHA256_Init(&context_);

}

void CryptoHelper::Hash::update(const std::string& data) {
    SHA256_Update(&context_, data.c_str(), data.length());
}

hash_t CryptoHelper::Hash::finalize() {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &context_);
    return hash_helper::fromArray(hash);
}