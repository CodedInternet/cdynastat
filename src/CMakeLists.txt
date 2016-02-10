# Build the main executable
add_definitions(-DWEBRTC_POSIX)

add_library(AbstractDynastat AbstractDynastat.cpp)
add_library(DynastatSimulator DynastatSimulator.cpp)
target_link_libraries(DynastatSimulator AbstractDynastat)
add_library(PeerConnectionClient PeerConnectionClient.cpp)
target_link_libraries(PeerConnectionClient ${OPENSSL_LIBRARIES})
#add_library(WebRTCConductor conductor.cpp conductor.h)

add_executable(cdynastat main.cpp conductor.cpp)
target_link_libraries(cdynastat ${LIBWEBRTC} ${X11_LIBRARIES} PeerConnectionClient DynastatSimulator)

add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
target_link_libraries(websocketpp_debug_client ${OPENSSL_LIBRARIES})
