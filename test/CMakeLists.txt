set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(LIBS pthread)
add_executable(test_log test_log.cpp)
target_link_libraries(test_log PRIVATE ${LIBS})