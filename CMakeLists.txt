cmake_minimum_required(VERSION 3.10)
project(Inquisitor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBPCAP REQUIRED libpcap)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PC_LIBPCAP_INCLUDE_DIRS}
)

# Add executable
add_executable(inquisitor
    src/main.cpp
    src/InquisitorFacade.cpp
    src/InquisitorFacade.hpp
    src/arp/ArpPoisoner.cpp
    src/arp/ArpPoisoner.hpp
    src/network/PacketCapture.cpp
    src/network/PacketCapture.hpp
    src/protocol/FtpParser.cpp
    src/protocol/FtpParser.hpp
)

# Link libraries
target_link_libraries(inquisitor
    ${PC_LIBPCAP_LIBRARIES}
    pthread
)

# Installation
install(TARGETS inquisitor DESTINATION bin)

# Enable testing
enable_testing()

# Add tests if any
# add_subdirectory(tests)
