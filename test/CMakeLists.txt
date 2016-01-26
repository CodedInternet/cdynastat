# Test definitions
add_executable(allTests ${SOURCE_FILES} AbstractDynastat_unittest.cpp)
target_link_libraries(allTests gtest_main gtest ${SHARED_LIBS})