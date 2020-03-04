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

#include "scn/CryptoHelper/CryptoHelper.h"
#include <gtest/gtest.h>

using namespace scn;

class TestCrypto : public testing::Test {
public:
    TestCrypto()
            :crypto_(example_owner_private_key_) { }

protected:

    public_key_t example_owner_public_key_ = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                          "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvY\n"
                                                          "JzyHUNUzT6IngeP4ijXcHHqTXyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                                          "-----END PUBLIC KEY-----");

    private_key_t example_owner_private_key_ = "-----BEGIN EC PRIVATE KEY-----\n"
                                               "MHQCAQEEIGm1P/iWsWSXlGCLSmokqRN3yKjx5HujGNjCkKOMF21poAcGBSuBBAAK\n"
                                               "oUQDQgAEvdfi1bMgqn03FuVcjwtLMJyfnxinHrvYJzyHUNUzT6IngeP4ijXcHHqT\n"
                                               "XyfEoqZ5Clz+ZlOSYL1beQTpJ4BDwg==\n"
                                               "-----END EC PRIVATE KEY-----";

    public_key_t other_public_key_ = PublicKeyPEM("-----BEGIN PUBLIC KEY-----\n"
                                                  "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEMoMRqM5hS3hc3oiocVvjOdlD61FiK/6V\n"
                                                  "CONcprHNBpbQCN52kLAnFVyE7wPu5rCIGspsQtC5zcKqhYidXa48Ew==\n"
                                                  "-----END PUBLIC KEY-----");

    CryptoHelper crypto_;
};


TEST_F(TestCrypto, HashCalculationShort) {
    auto hash = CryptoHelper::calcHash("A random text.");
    EXPECT_EQ(hash_helper::toString(hash), "1FF38799705BD4B5CEAA66EE3DEC54A304E16B64A1711838730EFF6752F2BA6");
}

TEST_F(TestCrypto, HashCalculationShortStream) {
    std::stringstream ss;
    ss << "A random text.";
    auto hash = CryptoHelper::calcHash(ss);
    EXPECT_EQ(hash_helper::toString(hash), "1FF38799705BD4B5CEAA66EE3DEC54A304E16B64A1711838730EFF6752F2BA6");
}

TEST_F(TestCrypto, HashCalculationShortBuffer) {
    const char* buffer = "A random text.";
    auto hash = CryptoHelper::calcHash(buffer, strlen(buffer));
    EXPECT_EQ(hash_helper::toString(hash), "1FF38799705BD4B5CEAA66EE3DEC54A304E16B64A1711838730EFF6752F2BA6");
}

void buildLongStringStream(std::stringstream& ss) {
    //about 100 MB
    for(auto i=0;i<7500000;i++) {
        ss << "A random text.";
    }
}

TEST_F(TestCrypto, HashCalculationLong) {
    std::stringstream ss;
    buildLongStringStream(ss);
    auto hash = CryptoHelper::calcHash(ss.str());
    EXPECT_EQ(hash_helper::toString(hash), "D428F80D2822CFF295B9746FA9D4D10B497EB3EC0D2B63A562425D69EAB9C5A7");
}

TEST_F(TestCrypto, HashCalculationLongStream) {
    std::stringstream ss;
    buildLongStringStream(ss);
    auto hash = CryptoHelper::calcHash(ss);
    EXPECT_EQ(hash_helper::toString(hash), "D428F80D2822CFF295B9746FA9D4D10B497EB3EC0D2B63A562425D69EAB9C5A7");
}


TEST_F(TestCrypto, SignatureCalculationValid) {
    auto signature = crypto_.calcSignature("A random text.");
    EXPECT_EQ(crypto_.verifySignature("A random text.", signature, example_owner_public_key_), true);
}

TEST_F(TestCrypto, SignatureCalculationInvalid1) {
    auto signature = crypto_.calcSignature("A random text.");
    EXPECT_EQ(crypto_.verifySignature("B random text.", signature, example_owner_public_key_), false);
}

TEST_F(TestCrypto, SignatureCalculationInvalid2) {
    auto signature = crypto_.calcSignature("A random text.");
    EXPECT_EQ(crypto_.verifySignature("A random text.", signature, other_public_key_), false);
}

TEST_F(TestCrypto, SignatureCalculationInvalid3) {
    auto signature = crypto_.calcSignature("A random text.");
    signature[7]++;
    EXPECT_EQ(crypto_.verifySignature("A random text.", signature, example_owner_public_key_), false);
}

TEST_F(TestCrypto, InvalidPrivateKey) {
    CryptoHelper crypto_local("ABC");
    auto signature = crypto_local.calcSignature("A random text.");
    EXPECT_TRUE(signature.empty());
}

TEST_F(TestCrypto, IsPublicKeyValid) {
    EXPECT_TRUE(CryptoHelper::isPublicKeyValid(example_owner_public_key_));
    EXPECT_FALSE(CryptoHelper::isPublicKeyValid(PublicKeyPEM()));
}

TEST_F(TestCrypto, IsPrivateKeyValid) {
    EXPECT_TRUE(CryptoHelper::isPrivateKeyValid(example_owner_private_key_));
    EXPECT_FALSE(CryptoHelper::isPrivateKeyValid("ABC"));
}