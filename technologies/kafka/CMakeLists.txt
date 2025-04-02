add_library(kafka_technology STATIC)

target_sources(kafka_technology PRIVATE
    KafkaPublisher.cpp
    KafkaConsumer.cpp
)

if(WIN32)
    # Use vcpkg paths on Windows
    set(KAFKA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows-static/include")
    set(KAFKA_LIB_DIR "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows-static/lib")
    target_link_libraries(kafka_technology PRIVATE
        core
        factories
        "${KAFKA_LIB_DIR}/rdkafka.lib"
    )
elseif(UNIX)
    # Use system paths on Ubuntu/Docker
    set(KAFKA_INCLUDE_DIR "/usr/include")
    set(KAFKA_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    target_link_libraries(kafka_technology PRIVATE
        core
        factories
        "${KAFKA_LIB_DIR}/librdkafka.a"
    )
endif()

target_include_directories(kafka_technology PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KAFKA_INCLUDE_DIR}
)

target_link_libraries(PublisherApp PRIVATE core factories kafka_technology ${KAFKA_LIB_DIR}/lz4.lib)
target_link_libraries(ConsumerApp PRIVATE core factories kafka_technology)

