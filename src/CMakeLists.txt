# Build the main executable
add_definitions(-DWEBRTC_POSIX)

add_library(AbstractDynastat AbstractDynastat.cpp AbstractDynastat.h)
add_library(DynastatSimulator DynastatSimulator.cpp DynastatSimulator.h)
add_library(PeerConnectionClient PeerConnectionClient.cpp PeerConnectionClient.h)

set(SOURCE_FILES conductor.cpp Dynastat.cpp)
add_executable(cdynastat main.cpp)
target_link_libraries(cdynastat ${SHARED_LIBS} AbstractDynastat DynastatSimulator PeerConnectionClient)

add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
target_link_libraries(websocketpp_debug_client)