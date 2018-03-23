include_directories(${PROJECT_SOURCE_DIR/rudp})
set(APP_SRC test_rf.cpp)
add_executable(test_rf ${APP_SRC})
target_link_libraries(test_rf librudp pthread)
