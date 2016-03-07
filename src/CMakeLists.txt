# Build the main executable
add_library(AbstractDynastat AbstractDynastat.cpp)
target_link_libraries(AbstractDynastat ${LIBWEBRTC})

add_library(DynastatSimulator DynastatSimulator.cpp)
target_link_libraries(DynastatSimulator AbstractDynastat)

add_library(PeerConnectionClient PeerConnectionClient.cpp)
target_link_libraries(PeerConnectionClient ${OPENSSL_LIBRARIES})

add_library(WebRTCConductor conductor.cpp conductor.h)
target_link_libraries(WebRTCConductor ${OPENSSL_LIBRARIES} ${LIBWEBRTC} ${X11_LIBRARIES})

add_executable(cdynastat main.cpp)
target_link_libraries(cdynastat WebRTCConductor DynastatSimulator PeerConnectionClient)

add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
target_link_libraries(websocketpp_debug_client ${OPENSSL_LIBRARIES})
