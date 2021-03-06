# Build the main executable
add_library(AbstractDynastat AbstractDynastat.cpp)
target_link_libraries(AbstractDynastat ${LIBWEBRTC})

add_library(Dynastat Dynastat.cpp)
target_link_libraries(Dynastat AbstractDynastat ${YAMLCPP_LIBRARY_RELEASE})

add_library(DynastatSimulator DynastatSimulator.cpp)
target_link_libraries(DynastatSimulator AbstractDynastat)

add_library(PeerConnectionClient PeerConnectionClient.cpp)
target_link_libraries(PeerConnectionClient ${OPENSSL_LIBRARIES} ssl crypto)

add_library(WebRTCConductor conductor.cpp conductor.h)
target_link_libraries(WebRTCConductor ${OPENSSL_LIBRARIES} ${LIBWEBRTC} ${X11_LIBRARIES} ${YAMLCPP_LIBRARY_RELEASE})

add_executable(cdynastat main.cpp)
target_link_libraries(cdynastat dl pthread WebRTCConductor Dynastat PeerConnectionClient)

#add_definitions(-DSIMULATOR)
#add_executable(simulator main.cpp)
#target_link_libraries(simulator dl pthread WebRTCConductor DynastatSimulator PeerConnectionClient)

add_executable(ByteTest ByteTest.cpp)

#add_executable(ControlTest ControlTest.cpp)
#target_link_libraries(ControlTest Dynastat)

#add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
#target_link_libraries(websocketpp_debug_client ${OPENSSL_LIBRARIES} ssl crypto )

add_executable(SensorAddress SensorAddress.cpp)
target_link_libraries(SensorAddress Dynastat)
