add_library(zeromq_p2p STATIC)

target_sources(zeromq_p2p PUBLIC
    ZeroMQP2PConsumer.cpp
    ZeroMQP2PPublisher.cpp
)

if(WIN32)
    # Use vcpkg paths on Windows
    set(ZMQ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows/include")
    set(ZMQ_LIB_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib")
    target_link_libraries(zeromq_p2p PRIVATE
        core
        "${ZMQ_LIB_DIR}/libzmq-mt-4_3_5.lib"
    )
elseif(UNIX)
    # Use system paths on Ubuntu/Docker
    set(ZMQ_INCLUDE_DIR "/usr/include")
    set(ZMQ_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    target_link_libraries(zeromq_p2p PRIVATE
        core
        "${ZMQ_LIB_DIR}/libzmq.so"
    )
endif()

target_include_directories(zeromq_p2p PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZMQ_INCLUDE_DIR}
)

target_link_libraries(PublisherApp PRIVATE zeromq_p2p)
target_link_libraries(ConsumerApp PRIVATE zeromq_p2p)

# target_include_directories(zeromq_consumer PRIVATE
#     ${CMAKE_BINARY_DIR}/core/interfaces
#     ${CMAKE_BINARY_DIR}/core/factory
#     ${ZMQ_INCLUDE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )

# target_link_directories(zeromq_consumer PRIVATE
#     ${CMAKE_BINARY_DIR}/../vcpkg/installed/x64-windows/lib
# )

# # Fix ZeroMQ linking
# if (MINGW)
#     target_link_libraries(zeromq_consumer PRIVATE
#         core
#         ${SPDLOG_LIB} 
#         ${FMT_LIB}
#         "${ZMQ_LIB_DIR}/libzmq-mt-4_3_5.lib"
#     )
# else()
#     target_link_libraries(zeromq_consumer PRIVATE
#         core
#         ${SPDLOG_LIB} 
#         ${FMT_LIB}
#         "${ZMQ_LIB_DIR}/libzmq.so"
#     )
# endif()

# # Add pkg-config flags manually (if needed)
# target_compile_options(zeromq_consumer PRIVATE ${ZeroMQ_CFLAGS_OTHER})
# target_link_options(zeromq_consumer PRIVATE ${ZeroMQ_LDFLAGS})

# # Add compiler flags
# # if (MSVC)
# #     target_compile_options(zeromq_consumer PRIVATE /W4)
# # else()
# #     target_compile_options(zeromq_consumer PRIVATE -Wall -Wextra -Werror)
# # endif()
