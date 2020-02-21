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

#include "PublicKeyPEM.h"
#include <sstream>
#include <algorithm>

using namespace scn;

const std::string PublicKeyPEM::prefix_ = "-----BEGIN PUBLIC KEY-----\n";
const std::string PublicKeyPEM::postfix_ = "\n-----END PUBLIC KEY-----";


PublicKeyPEM::PublicKeyPEM(const std::ifstream& public_key_file_stream) {
    std::stringstream public_key_stringstream;
    public_key_stringstream << public_key_file_stream.rdbuf();
    initFromString(public_key_stringstream.str());
}


PublicKeyPEM::PublicKeyPEM(const std::string& public_key_string) {
    initFromString(public_key_string);
}


PublicKeyPEM::PublicKeyPEM()
:short_string_() {

}


PublicKeyPEM::~PublicKeyPEM() = default;


void PublicKeyPEM::initFromString(const std::string &public_key_string) {
    //read part between prefix and postfix to minimum_string_
    auto start_index = public_key_string.find(prefix_);
    auto end_index = public_key_string.find(postfix_);
    if(start_index == std::string::npos || end_index == std::string::npos) {
        short_string_ = "";
        return;
    }
    start_index += prefix_.length();
    if(start_index > end_index) {
        short_string_ = "";
        return;
    }
    short_string_ = public_key_string.substr(start_index, end_index-start_index+1);

    //remove blanks, tabs, line breaks
    short_string_.erase(std::remove(short_string_.begin(), short_string_.end(), ' '), short_string_.end());
    short_string_.erase(std::remove(short_string_.begin(), short_string_.end(), '\t'), short_string_.end());
    short_string_.erase(std::remove(short_string_.begin(), short_string_.end(), '\n'), short_string_.end());
    short_string_.erase(std::remove(short_string_.begin(), short_string_.end(), '\r'), short_string_.end());
}


std::string PublicKeyPEM::getAsFullString() const {
    return prefix_ + short_string_ + postfix_;
}


std::string PublicKeyPEM::getAsShortString() const {
    return short_string_;
}


bool PublicKeyPEM::isEmpty() const {
    return short_string_.length() == 0;
}