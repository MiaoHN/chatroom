set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(LIBS pthread sqlite3)

set(FILES test_log test_database)

foreach(FILE ${FILES})
  add_executable(${FILE} ${FILE}.cpp)
  target_link_libraries(${FILE} PRIVATE ${LIBS})
endforeach(FILE ${FILES})
