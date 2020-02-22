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

#include <iostream>
#include <git_info.h>

#include "scn/Miner/MinerLocal.h"
#include "scn/Blockchain/Blockchain.h"
#include "scn/P2PConnector/P2PConnector.h"
#include "scn/BlockchainManager/BlockchainManager.h"
#include "scn/SystemMonitor/SystemMonitor.h"
#include <chrono>


int startCommandLineApp(int argc, char* argv[]);

int main(int argc, char* argv[]) {

    google::InitGoogleLogging(argv[0]);

    return startCommandLineApp(argc, argv);
}

#ifdef _WIN32
enum class ConsoleColor : WORD
{
    WHITE = 0x07,
    GREEN = 0x02,
    YELLOW = 0x0e,
    RED = 0x0c
};
HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
template <class T>
void printColored(const T& content, ConsoleColor color)
{
    CONSOLE_SCREEN_BUFFER_INFO con_info;
    GetConsoleScreenBufferInfo(h_console, &con_info);
    SetConsoleTextAttribute(h_console, static_cast<WORD>(color));
    std::cout << content;
    SetConsoleTextAttribute(h_console, con_info.wAttributes);
}
#else
enum class ConsoleColor : uint32_t
{
    WHITE = 37,
    GREEN = 32,
    YELLOW = 33,
    RED = 31
};
template <class T>
void printColored(const T& content, ConsoleColor color)
{
    std::cout << "\033[1;" << static_cast<uint32_t>(color) << "m" << content << "\033[0m";
}
#endif

int startCommandLineApp(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " PUBLIC_KEY_FILE PRIVATE_KEY_FILE NUM_MINING_THREADS LISTEN_PORT" << std::endl;
        return 1;
    }

    std::srand(std::time(nullptr));

    std::ifstream public_key_stream(argv[1]);
    scn::public_key_t public_key(public_key_stream);
    std::ifstream private_key_stream(argv[2]);
    std::stringstream private_key_string;
    private_key_string << private_key_stream.rdbuf();

    scn::Blockchain blockchain("./blockchains/" + std::string(argv[4]) + "/");
    scn::P2PConnector p2p_connector(std::stoi(std::string(argv[4])), blockchain);
    scn::MinerLocal miner(std::stoi(std::string(argv[3])));
    scn::BlockchainManager manager(public_key, private_key_string.str(), blockchain, p2p_connector, miner);
    scn::SystemMonitor system_monitor;

    while(true) {
        std::cout <<
            "******************************  SwabianCoin  ******************************" << std::endl <<
            "[t] new transaction   [s] status   [q] quit   [p] peers" << std::endl <<
            "[v] git info   [mX] mining threads" << std::endl;

        char control_character;
        std::cin >> control_character;

        switch(control_character) {
            case 't':
            {
                std::cout << "Please type the public key filename of the receiver:" << std::endl;
                scn::public_key_t receiver_public_key;
                std::string receiver_public_key_file;
                std::cin >> receiver_public_key_file;
                if(receiver_public_key_file.length() > 0) {
                    std::ifstream receiver_public_key_stream(receiver_public_key_file);
                    receiver_public_key = scn::public_key_t(receiver_public_key_stream);
                }
                if(!scn::CryptoHelper::isPublicKeyValid(receiver_public_key)) {
                    std::cout << "Invalid public key file" << std::endl;
                    break;
                }
                std::cout << "Please type the amount of swabian coins to transfer to the receiver:" << std::endl;
                double amount;
                std::cin >> amount;

                std::cout << "transfer " << amount << " to " << receiver_public_key.getAsShortString() << std::endl << std::endl;
                manager.triggerTransaction(receiver_public_key, static_cast<uint64_t>(amount * static_cast<double>(scn::TransactionSubBlock::fraction_per_coin)));
                break;
            }
            case 'm':
            {
                uint32_t num_worker_threads;
                std::cin >> num_worker_threads;
                miner.changeNumWorkerThreads(num_worker_threads);
                ConsoleColor c = miner.isRunning() ? (num_worker_threads > 0 ? ConsoleColor::GREEN : ConsoleColor::YELLOW) : ConsoleColor::RED;
                std::cout << "Miner: ";
                miner.isRunning() ? printColored("running (threads: " + std::to_string(num_worker_threads) + ")", c) : printColored("not running", c);
                std::cout << std::endl << std::endl;
                break;
            }
            case 's':
            {
                auto block = blockchain.getNewestBlock();
                auto mining_state = blockchain.getMiningState();
                auto num_peers = p2p_connector.numConnectedPeers();
                auto percent_synchronized = manager.percentBlockchainSynchronized();
                auto num_worker_threads = miner.numWorkerThreads();
                auto cps = miner.numChecksPerSecond();
                ConsoleColor miner_c = miner.isRunning() ? (num_worker_threads > 0 ? ConsoleColor::GREEN : ConsoleColor::YELLOW) : ConsoleColor::RED;
                std::cout << "Newest Block Index: " << block->header.block_uid << std::endl <<
                    "Newest Block Hash: " << scn::hash_helper::toString(block->header.generic_header.block_hash) << std::endl <<
                    "Balance: " << std::fixed << std::setprecision(6) << static_cast<double>(blockchain.getBalance(public_key)) /
                    static_cast<double>(scn::TransactionSubBlock::fraction_per_coin) << std::endl <<
                    "Peers: ";
                printColored(num_peers, num_peers > 0 ? ConsoleColor::GREEN : ConsoleColor::YELLOW);
                std::cout << "\t\t" << "Miner: ";
                miner.isRunning() ? printColored("running (threads: " + std::to_string(num_worker_threads) + " cps: " + std::to_string(cps) + ")", miner_c) : printColored("not running", miner_c);
                std::cout << "\t" << "Blockchain: ";
                percent_synchronized == 100 ? printColored("synchronized", ConsoleColor::GREEN) : printColored("synchronizing (" + std::to_string(percent_synchronized) + "%)", ConsoleColor::YELLOW);
                std::cout << std::endl << "Epoch: " << mining_state.epoch << "\t\tCoins in circulation: " << (mining_state.num_minings_in_epoch + mining_state.epoch * scn::CollectionBlock::max_num_creations) << std::endl << std::endl;
                break;
            }
            case 'p':
            {
                std::cout << "Peers [" << p2p_connector.numConnectedPeers() << "]:" << std::endl;
                p2p_connector.printPeerInfo();
                std::cout << std::endl;
                break;
            }
            case 'q':
            {
                return 0;
                break;
            }
            case 'v':
            {
                if(GIT_RETRIEVED_STATE) {
                    std::cout << "Git revision: " << GIT_HEAD_SHA1 << std::endl;
                    std::cout << "Clean Build:  " << (GIT_IS_DIRTY ? "NO" : "YES") << std::endl << std::endl;
                } else {
                    std::cout << "No git information found" << std::endl << std::endl;
                }
                break;
            }
            default:
                break;
        }
    }

    return 0;
}