add_library(core-interfaces INTERFACE)
target_include_directories(core-interfaces INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Create a static library for core components
add_library(core STATIC)

# Add source files for logger, factory, and interfaces?
target_sources(core PRIVATE 
        logger/Logger.cpp
        factory/ConsumerFactory.cpp
)

# Add public headers so they can be included by other targets
target_include_directories(core 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/logger
        ${CMAKE_CURRENT_SOURCE_DIR}/factory
)

target_link_libraries(core PUBLIC 
    core-interfaces
)

# Make this library available for use in other projects
set_target_properties(core PROPERTIES EXPORT_NAME Core)

