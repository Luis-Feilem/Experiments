add_executable(ConsumerApp ConsumerApp.cpp)
target_link_libraries(ConsumerApp PUBLIC core)


add_executable(PublisherApp PublisherApp.cpp)
target_link_libraries(PublisherApp PUBLIC core)