# Test definitions
include_directories(../src)
add_executable(allTests AbstractDynastat_unittest.cpp PeerConnectionClient_unittest.cpp)
# Make sure OpenSSL gets linked before WebRTC
target_link_libraries(allTests gtest gtest_main gmock PeerConnectionClient AbstractDynastat)

add_test(PeerConnectionClientTest allTests --gtest_filter=PeerConnectionClientTest.*)
add_test(AbstractSensorTest allTests --gtest_filter=SensorTest.*:)
