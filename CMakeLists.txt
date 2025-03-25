cmake_minimum_required(VERSION 3.16)
project(MessagingBenchmark)

# message(STATUS "ROOT: Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# # Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific setup
if (WIN32)
    message(STATUS "ROOT: Building on Windows")
    # vcpkg integrates automatically with CMake if the toolchain file is set
    set(CMAKE_TOOLCHAIN_FILE "../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif (UNIX)
    message(STATUS "ROOT: Building on Linux")
endif()

# Add subdirectories
add_subdirectory(core)
add_subdirectory(core/applications)

# add as technologies are added (this is UGLY but whatever)
if(EXISTS "${CMAKE_SOURCE_DIR}/technologies/zeromq_p2p")
    message(STATUS "Including ZeroMQ P2P technology")
    add_subdirectory(technologies/zeromq_p2p)
else()
    message(STATUS "Skipping ZeroMQ P2P technology (not found)")
endif()

# target_link_libraries(MessagingBenchmark PRIVATE core)