# Test definitions
include_directories(../src)
SET(TEST_SOURCES ${SOURCE_DIR}/AbstractDynastat.cpp ${SOURCE_DIR}/PeerConnectionClient.cpp)
add_executable(allTests AbstractDynastat_unittest.cpp PeerConnectionClient_unittest.cpp ${TEST_SOURCES})
target_link_libraries(allTests gtest gtest_main gmock ${LIBWEBRTC} ${X11_LIBRARIES} ${OPENSSL_LIBRARIES})

add_test(PeerConnectionClientTest allTests --gtest_filter=PeerConnectionClientTest.*:PeerConnectionClientTest/*.*:*/PeerConnectionClientTest.*/*:*/PeerConnectionClientTest/*.*)

