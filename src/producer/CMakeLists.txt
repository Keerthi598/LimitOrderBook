add_executable(producerApp producerMain.cpp)

target_link_libraries(
        producerApp
        PRIVATE
            ipc_protocol
)
