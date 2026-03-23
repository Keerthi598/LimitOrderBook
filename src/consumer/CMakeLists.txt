add_executable(consumerApp consumerMain.cpp)

target_link_libraries(
        consumerApp
        PUBLIC
            ipc_protocol
            matching_engine
)
