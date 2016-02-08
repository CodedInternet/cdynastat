# Build the main executable
add_definitions(-DWEBRTC_POSIX)

#add_library(AbstractDynastat AbstractDynastat.cpp AbstractDynastat.h)
#add_library(DynastatSimulator DynastatSimulator.cpp DynastatSimulator.h)
#target_link_libraries(DynastatSimulator AbstractDynastat)
#add_library(PeerConnectionClient PeerConnectionClient.cpp PeerConnectionClient.h)
#add_library(WebRTCConductor conductor.cpp conductor.h)

set(SOURCE_FILES AbstractDynastat.cpp DynastatSimulator.cpp PeerConnectionClient.cpp conductor.cpp )
add_executable(cdynastat main.cpp ${SOURCE_FILES})
target_link_libraries(cdynastat ${LIBWEBRTC} ${X11_LIBRARIES} ${OPENSSL_LIBRARIES})

add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
target_link_libraries(websocketpp_debug_client ${OPENSSL_LIBRARIES})

add_executable(conductor_debug conductor_debug.cpp conductor.cpp DynastatSimulator.cpp AbstractDynastat.cpp)
target_link_libraries(conductor_debug ${LIBWEBRTC} ${X11_LIBRARIES} ${OPENSSL_LIBRARIES})
