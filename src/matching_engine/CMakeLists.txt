add_library(matching_engine matchingEngine.cpp orderPool.cpp)

target_include_directories(
        matching_engine
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
        matching_engine
        PRIVATE
        ipc_protocol
)