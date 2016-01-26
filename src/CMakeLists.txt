# Build the main executable

set(SOURCE_FILES conductor.cpp Dynastat.cpp AbstractDynastat.cpp DynastatSimulator.cpp)
add_executable(cdynastat main.cpp ${SOURCE_FILES})
target_link_libraries(cdynastat ${SHARED_LIBS})