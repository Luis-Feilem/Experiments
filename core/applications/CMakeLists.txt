add_executable(ConsumerApp ConsumerApp.cpp)

# target_include_directories(ConsumerApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# get_target_property(core_INCLUDES core INTERFACE_INCLUDE_DIRECTORIES)
# message(STATUS "[applications]: include directories: ${core_INCLUDES}")

target_link_libraries(ConsumerApp PUBLIC core)

# add_executable(Publisherapp PublisherApp.cpp)
# target_link_libraries(PublisherApp PRIVATE core)
