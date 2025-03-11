cmake_minimum_required(VERSION 3.12)
project(MessagingBenchmark)

# Use C++17 for modern language features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ensure vcpkg toolchain is applied (if not already set via preset)
if (NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Only add subdirectory for Windows builds (local)
if(WIN32)
    add_subdirectory(technologies/zeromq)
endif()
