cmake_minimum_required(VERSION 3.12)

# Convert vcpkg install path to absolute path
get_filename_component(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows" ABSOLUTE)

# Ensure ZeroMQ headers and libraries are included
set(ZMQ_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/include")
set(ZMQ_LIB_DIR "${VCPKG_INSTALLED_DIR}/lib")

# Normalize Windows paths (handles both / and \)
file(TO_CMAKE_PATH "${ZMQ_INCLUDE_DIR}" ZMQ_INCLUDE_DIR)
file(TO_CMAKE_PATH "${ZMQ_LIB_DIR}" ZMQ_LIB_DIR)

# Debugging output (after resolving absolute paths)
message(STATUS "ZeroMQ Include Dir: ${ZMQ_INCLUDE_DIR}")
message(STATUS "ZeroMQ Lib Dir: ${ZMQ_LIB_DIR}")

# Find nlohmann-json package
find_package(nlohmann_json CONFIG REQUIRED)

# Create a library for ZeroMQ publisher and consumer
add_library(ZeroMQLib
    ZeroMQPublisher.cpp
    ZeroMQConsumer.cpp
)

# Ensure the library includes necessary headers
target_include_directories(ZeroMQLib PRIVATE ../../core/interfaces ${ZMQ_INCLUDE_DIR})
target_compile_options(ZeroMQLib PRIVATE "-I${ZMQ_INCLUDE_DIR}")

# Link ZeroMQ correctly
target_link_libraries(ZeroMQLib PRIVATE "${ZMQ_LIB_DIR}/libzmq-mt-4_3_5.lib")

# Static linking for libstdc++ and libgcc
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Create executable for Publisher App
add_executable(ZeroMQPublisherApp ZeroMQPublisherApp.cpp)
target_include_directories(ZeroMQPublisherApp PRIVATE ${ZMQ_INCLUDE_DIR})
target_link_libraries(ZeroMQPublisherApp PRIVATE ZeroMQLib nlohmann_json::nlohmann_json)

# Create executable for Consumer App
add_executable(ZeroMQConsumerApp ZeroMQConsumerApp.cpp)
target_include_directories(ZeroMQConsumerApp PRIVATE ${ZMQ_INCLUDE_DIR})
target_link_libraries(ZeroMQConsumerApp PRIVATE ZeroMQLib nlohmann_json::nlohmann_json)
