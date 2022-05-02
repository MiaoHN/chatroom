set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/server)

add_executable(server server.cpp main.cpp)
target_link_libraries(server PRIVATE chatroom)