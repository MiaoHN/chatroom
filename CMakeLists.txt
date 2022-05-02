cmake_minimum_required(VERSION 3.16)
project(Chatroom)

include_directories(include)

link_directories(lib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
file(GLOB LIB_SRC 
  "src/*.cpp")
set(LIB_LIB sqlite3 pthread)
add_library(chatroom ${LIB_SRC})
target_link_libraries(chatroom PRIVATE ${LIB_LIB})

add_subdirectory(test)
add_subdirectory(server)
add_subdirectory(client)