cmake_minimum_required(VERSION 3.10)
project(full_node)

option(BUILD_UNIT_TESTS "build unit tests" OFF)

set(CMAKE_CXX_STANDARD 14)

set(OPENSSL_USE_STATIC_LIBS FALSE)
find_package(OpenSSL REQUIRED)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.58 REQUIRED
        system
        filesystem
        iostreams)

find_package(CURL REQUIRED)

include_directories(src/
        dep/libtorrent-rasterbar-1.2.2_extended/include/
        dep/cereal-1.3.0/include/
        dep/curl-7.64.1/include/
        cmake/include/
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS})

add_library(full_node_library
        src/scn/Common/Common.cpp
        src/scn/Common/BloomFilter.cpp
        src/scn/Common/PublicKeyPEM.cpp
        src/scn/Miner/MinerLocal.cpp
        src/scn/Blockchain/Blockchain.cpp
        src/scn/Blockchain/BlockDefinitions.cpp
        src/scn/Blockchain/Cache.cpp
        src/scn/BlockchainManager/BlockchainManager.cpp
        src/scn/BlockchainManager/CycleStateFetchBlockchain.cpp
        src/scn/BlockchainManager/CycleStateCollect.cpp
        src/scn/BlockchainManager/CycleStateIntroduceBlock.cpp
        src/scn/BlockchainManager/CycleStateIntroduceBaseline.cpp
        src/scn/BlockchainManager/OutOfSyncDetector.cpp
        src/scn/BlockchainManager/PeersMonitor.cpp
        src/scn/BlockchainManager/ActivePeersCollector.cpp
        src/scn/CryptoHelper/CryptoHelper.cpp
        src/scn/CryptoHelper/HashStreamBuf.cpp
        src/scn/P2PConnector/P2PConnector.cpp
        src/scn/P2PConnector/EntryPointFetcher.cpp
        src/scn/SynchronizedTime/SynchronizedTimer.cpp
        src/scn/SystemMonitor/SystemMonitor.cpp
        )

set(BUILD_TESTING OFF)
add_subdirectory(dep/glog-0.4.0)
add_subdirectory(dep/libtorrent-rasterbar-1.2.2_extended)
target_link_libraries(full_node_library
        glog::glog
        ${Boost_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${CURL_LIBRARIES}
        torrent-rasterbar
        )

target_compile_options(full_node_library PRIVATE -Wall -rdynamic)
target_link_options(full_node_library PRIVATE -rdynamic)
target_compile_definitions(full_node_library PRIVATE CEREAL_SERIALIZE_FUNCTION_NAME=ser)

set(PRE_CONFIGURE_FILE "cmake/git_info.h.in")
set(POST_CONFIGURE_FILE "cmake/include/git_info.h")
include(cmake/git_watcher.cmake)

add_executable(full_node_cli
        main.cpp
        )

add_dependencies(full_node_cli
        check_git_repository
        )

target_link_libraries(full_node_cli
        full_node_library
        )

target_compile_options(full_node_cli PRIVATE -Wall -rdynamic)
target_link_options(full_node_cli PRIVATE -rdynamic)
target_compile_definitions(full_node_cli PRIVATE CEREAL_SERIALIZE_FUNCTION_NAME=ser)

if(BUILD_UNIT_TESTS)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_executable(runTests
            test/TestCrypto.cpp
            test/TestSerialization.cpp
            test/TestBlockchainManager.cpp
            test/TestMiner.cpp
            test/TestBlockchain.cpp
            test/TestP2PConnector.cpp
            test/TestBloomFilter.cpp
            )
    target_link_libraries(runTests full_node_library ${GTEST_LIBRARIES} pthread)
    target_compile_definitions(runTests PRIVATE CEREAL_SERIALIZE_FUNCTION_NAME=ser)
endif()

install(TARGETS full_node_cli DESTINATION bin)