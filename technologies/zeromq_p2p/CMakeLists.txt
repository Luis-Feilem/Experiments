cmake_minimum_required(VERSION 3.12)

# Platform-specific settings
if(WIN32)
    # Use vcpkg paths on Windows
    set(ZMQ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows/include")
    set(ZMQ_LIB_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib")
elseif(UNIX)
    # Use system paths on Ubuntu/Docker
    set(ZMQ_INCLUDE_DIR "/usr/include")
    set(ZMQ_LIB_DIR "/usr/lib/x86_64-linux-gnu")
endif()

# Debugging output
message(STATUS "ZeroMQ Include Dir: ${ZMQ_INCLUDE_DIR}")
message(STATUS "ZeroMQ Lib Dir: ${ZMQ_LIB_DIR}")

# Find nlohmann-json package
find_package(nlohmann_json CONFIG REQUIRED)

# Create a library for ZeroMQ publisher and consumer
add_library(ZeroMQLib
    ZeroMQP2PPublisher.cpp
    ZeroMQP2PConsumer.cpp
    ../../core/interfaces/IPublisherApp.cpp
    ../../core/interfaces/IConsumerApp.cpp
)

# Ensure the library includes necessary headers
target_include_directories(ZeroMQLib PRIVATE ../../core/interfaces ${ZMQ_INCLUDE_DIR})
target_compile_options(ZeroMQLib PRIVATE "-I${ZMQ_INCLUDE_DIR}")

# Link ZeroMQ correctly
if(WIN32)
    target_link_libraries(ZeroMQLib PRIVATE "${ZMQ_LIB_DIR}/libzmq-mt-4_3_5.lib")
elseif(UNIX)
    target_link_libraries(ZeroMQLib PRIVATE "${ZMQ_LIB_DIR}/libzmq.so")
endif()


# Static linking for libstdc++ and libgcc
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Create executable for Publisher App
add_executable(ZeroMQPublisherAppP2P ZeroMQP2PPublisherApp.cpp)
target_include_directories(ZeroMQPublisherAppP2P PRIVATE ${ZMQ_INCLUDE_DIR})
target_link_libraries(ZeroMQPublisherAppP2P PRIVATE ZeroMQLib nlohmann_json::nlohmann_json)

# Create executable for Consumer App
add_executable(ZeroMQConsumerAppP2P ZeroMQP2PConsumerApp.cpp)
target_include_directories(ZeroMQConsumerAppP2P PRIVATE ${ZMQ_INCLUDE_DIR})
target_link_libraries(ZeroMQConsumerAppP2P PRIVATE ZeroMQLib nlohmann_json::nlohmann_json)

# Register executables as top-level targets (critical step!)
add_custom_target(build-zeromqp2p-publisher DEPENDS ZeroMQPublisherAppP2P)
add_custom_target(build-zeromqp2p-consumer DEPENDS ZeroMQConsumerAppP2P)