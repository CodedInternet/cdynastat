# Test definitions
include_directories(../src)
add_executable(allTests AbstractDynastat_unittest.cpp PeerConnectionClient_unittest.cpp)
target_link_libraries(allTests gtest_main gtest ${SHARED_LIBS} AbstractDynastat PeerConnectionClient)