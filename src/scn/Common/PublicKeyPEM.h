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

#ifndef FULL_NODE_PUBLICKEYPEM_H
#define FULL_NODE_PUBLICKEYPEM_H

#include <string>
#include <fstream>

namespace scn {

    class PublicKeyPEM {
    public:
        explicit PublicKeyPEM(const std::ifstream &public_key_file_stream);

        explicit PublicKeyPEM(const std::string &public_key_string);

        PublicKeyPEM();

        virtual ~PublicKeyPEM();

        const std::string getAsFullString() const;

        const std::string getAsShortString() const;

        bool isEmpty() const;

        template<class Archive>
        void ser(Archive& ar) {
            ar & short_string_;
        }

    protected:
        static const std::string prefix_;
        static const std::string postfix_;

        std::string short_string_;

        void initFromString(const std::string &public_key_string);
    };

    inline bool operator==(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs) {
        return lhs.getAsShortString() == rhs.getAsShortString();
    }

    inline bool operator!=(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs){
        return !(lhs == rhs);
    }

    inline bool operator<(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs) {
        return lhs.getAsShortString() < rhs.getAsShortString();
    }

    inline bool operator>(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs) {
        return lhs.getAsShortString() > rhs.getAsShortString();
    }

    inline bool operator<=(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs) {
        return lhs.getAsShortString() <= rhs.getAsShortString();
    }

    inline bool operator>=(const PublicKeyPEM& lhs, const PublicKeyPEM& rhs) {
        return lhs.getAsShortString() >= rhs.getAsShortString();
    }
}

#endif //FULL_NODE_PUBLICKEYPEM_H
