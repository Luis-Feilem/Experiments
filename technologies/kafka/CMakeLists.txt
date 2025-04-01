add_library(kafka_technology STATIC)

target_sources(kafka_technology PRIVATE
    KafkaPublisher.cpp
    KafkaConsumer.cpp
)

if(WIN32)
    # Use vcpkg paths on Windows
    set(CPPKAFKA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows-static/include")
    set(CPPKAFKA_LIB_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows-static/lib")
    target_link_libraries(kafka_technology PRIVATE
        core
        "${CPPKAFKA_LIB_DIR}/cppkafka.lib"
        "${CPPKAFKA_LIB_DIR}/rdkafka.lib"
        "${CPPKAFKA_LIB_DIR}/rdkafka++.lib"
    )
elseif(UNIX)
    # Use system paths on Ubuntu/Docker
    set(CPPKAFKA_INCLUDE_DIR "/usr/include")
    set(CPPKAFKA_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    target_link_libraries(kafka_technology PRIVATE
        core
        "${CPPKAFKA_LIB_DIR}/libcppkafka.a"
        "${CPPKAFKA_LIB_DIR}/librdkafka.a"
        "${CPPKAFKA_LIB_DIR}/librdkafka++.a"
    )
endif()

target_include_directories(kafka_technology PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CPPKAFKA_INCLUDE_DIR}
)

target_link_libraries(PublisherApp PRIVATE kafka_technology)
target_link_libraries(ConsumerApp PRIVATE kafka_technology)

target_compile_definitions(kafka_technology PRIVATE CPPKAFKA_STATIC)

