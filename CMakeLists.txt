cmake_minimum_required(VERSION 3.10)
project(DeriConsole)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable ccache for faster builds (if available)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

# Find fmt library
find_package(fmt REQUIRED)

# Fetch WebSocket++ library
include(FetchContent)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(websocketpp)

# Find Boost and OpenSSL
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(DeriConsole
    dericonsole.cpp
    src/webSocketClient.cpp
    src/deriapi.cpp
    src/utils.cpp
)

# Include directories
target_include_directories(DeriConsole PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${websocketpp_SOURCE_DIR}
    src
)

# Link libraries
target_link_libraries(DeriConsole PRIVATE
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
)
add_subdirectory(json)
target_link_libraries(DeriConsole PRIVATE nlohmann_json::nlohmann_json)
# Add definitions for Boost.Asio (optional, but recommended)
target_compile_definitions(DeriConsole PRIVATE
    BOOST_ASIO_STANDALONE=0
)