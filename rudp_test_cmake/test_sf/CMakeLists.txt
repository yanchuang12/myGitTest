include_directories(${PROJECT_SOURCE_DIR/rudp})
set(APP_SRC test_sf.cpp)
add_executable(test_sf ${APP_SRC})
target_link_libraries(test_sf librudp pthread)
