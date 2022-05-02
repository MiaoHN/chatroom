set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/client)

add_executable(client client.cpp main.cpp)
target_link_libraries(client PRIVATE chatroom)