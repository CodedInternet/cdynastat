# Build the main executable
add_definitions(-DWEBRTC_POSIX)
set(SOURCE_FILES conductor.cpp Dynastat.cpp AbstractDynastat.cpp DynastatSimulator.cpp PeerConnectionClient.cpp)
add_executable(cdynastat main.cpp ${SOURCE_FILES})
target_link_libraries(cdynastat ${SHARED_LIBS})

add_executable(websocketpp_debug_client websocketpp_debug_client.cpp)
target_link_libraries(websocketpp_debug_client)