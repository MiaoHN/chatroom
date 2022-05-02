set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(LIBS pthread sqlite3)

file(GLOB FILES "*.cpp")

foreach(FILE ${FILES})
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${FILE})  
  add_executable(${FILE_NAME} ${FILE})
  target_link_libraries(${FILE_NAME} PRIVATE ${LIBS})
endforeach(FILE ${FILES})