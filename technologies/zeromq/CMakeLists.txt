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

# Create a library for ZeroMQ publisher and consumer
add_library(ZeroMQLib
    ZeroMQPublisher.cpp
    ZeroMQConsumer.cpp
)

# Ensure the library includes necessary headers
target_include_directories(ZeroMQLib PRIVATE ../../core/interfaces ${ZMQ_INCLUDE_DIR})

# Force the compiler to recognize the include path
target_compile_options(ZeroMQLib PRIVATE "-I${ZMQ_INCLUDE_DIR}")

# Link ZeroMQ correctly
target_link_libraries(ZeroMQLib PRIVATE "${ZMQ_LIB_DIR}/libzmq-mt-4_3_5.lib")

# Create test executables
add_executable(ZeroMQPublisherTest ZeroMQPublisherTest.cpp)
add_executable(ZeroMQConsumerTest ZeroMQConsumerTest.cpp)

# Ensure the test executables also inherit the include path
target_include_directories(ZeroMQPublisherTest PRIVATE ${ZMQ_INCLUDE_DIR})
target_include_directories(ZeroMQConsumerTest PRIVATE ${ZMQ_INCLUDE_DIR})

# Link test executables to ZeroMQLib
target_link_libraries(ZeroMQPublisherTest PRIVATE ZeroMQLib)
target_link_libraries(ZeroMQConsumerTest PRIVATE ZeroMQLib)
