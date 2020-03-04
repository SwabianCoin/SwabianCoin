cmake_minimum_required(VERSION 3.10)
project(full_node)

option(BUILD_UNIT_TESTS "build unit tests" OFF)
option(UNIT_TEST_COVERAGE "create unit test code coverage report" OFF)

set(CMAKE_CXX_STANDARD 14)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set(OPENSSL_USE_STATIC_LIBS FALSE)
find_package(OpenSSL REQUIRED)

if(MSVC)
    set(Boost_USE_STATIC_LIBS       OFF)
else()
    set(Boost_USE_STATIC_LIBS       ON)
endif()
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.58 REQUIRED
        system
        filesystem
        iostreams)

include_directories(src/
        dep/libtorrent-rasterbar-1.2.2_extended/include/
        dep/cereal-1.3.0/include/
        dep/cpp-httplib-0.5.3/
        cmake/include/
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR})

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
        src/scn/BlockchainManager/BlockFetchAgent.cpp
        src/scn/CryptoHelper/CryptoHelper.cpp
        src/scn/CryptoHelper/HashStreamBuf.cpp
        src/scn/P2PConnector/P2PConnector.cpp
        src/scn/P2PConnector/EntryPointFetcher.cpp
        src/scn/SynchronizedTime/SynchronizedTimer.cpp
        src/scn/SystemMonitor/SystemMonitor.cpp
        )

set(BUILD_TESTING OFF)
add_subdirectory(dep/glog-0.4.0)
if(MSVC)
    set(BUILD_SHARED_LIBS OFF)
    set(static_runtime OFF)
endif()
add_subdirectory(dep/libtorrent-rasterbar-1.2.2_extended)
target_link_libraries(full_node_library
        glog::glog
        ${Boost_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARY}
        torrent-rasterbar
        )

if(NOT MSVC)
    target_compile_options(full_node_library PRIVATE -Wall)
endif()
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

if(NOT MSVC)
    target_compile_options(full_node_cli PRIVATE -Wall)
endif()
target_compile_definitions(full_node_cli PRIVATE CEREAL_SERIALIZE_FUNCTION_NAME=ser)

if(BUILD_UNIT_TESTS)
    set(INSTALL_GTEST OFF)
    add_subdirectory(dep/googletest-release-1.10.0)
    add_executable(runTests
            test/TestCrypto.cpp
            test/TestSerialization.cpp
            test/TestBlockchainManager.cpp
            test/TestPeersMonitor.cpp
            test/TestMiner.cpp
            test/TestBlockchain.cpp
            test/TestP2PConnector.cpp
            test/TestBloomFilter.cpp
            )
    target_link_libraries(runTests full_node_library ${GTEST_LIBRARIES} gtest)
    target_compile_definitions(runTests PRIVATE CEREAL_SERIALIZE_FUNCTION_NAME=ser)

    if(UNIT_TEST_COVERAGE)
        include(cmake/CodeCoverage.cmake)
        append_coverage_compiler_flags()
        setup_target_for_coverage_lcov(
                NAME runTests-coverage-lcov
                EXECUTABLE runTests --gtest_output=xml:test_results.xml
                DEPENDENCIES runTests
                EXCLUDE "${PROJECT_SOURCE_DIR}/dep/*" "${PROJECT_SOURCE_DIR}/test/*" "${PROJECT_SOURCE_DIR}/main.cpp" "/usr/include/*"
        )
        setup_target_for_coverage_gcovr_xml(
                NAME runTests-coverage-gcovr-xml
                EXECUTABLE runTests --gtest_output=xml:test_results.xml
                DEPENDENCIES runTests
                EXCLUDE "${PROJECT_SOURCE_DIR}/dep/*" "${PROJECT_SOURCE_DIR}/test/*"
        )
        setup_target_for_coverage_gcovr_html(
                NAME runTests-coverage-gcovr-html
                EXECUTABLE runTests --gtest_output=xml:test_results.xml
                DEPENDENCIES runTests
                EXCLUDE "${PROJECT_SOURCE_DIR}/dep/*" "${PROJECT_SOURCE_DIR}/test/*"
        )
    endif()
endif()

install(TARGETS full_node_cli DESTINATION bin)