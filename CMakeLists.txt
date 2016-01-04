cmake_minimum_required(VERSION 3.3)
project(cdynastat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

IF(APPLE)
    #SET(GUI_TYPE MACOSX_BUNDLE)
    INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
    FIND_LIBRARY(CARBON_LIBRARY Carbon)
    FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime )
    FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices )
    MARK_AS_ADVANCED (CARBON_LIBRARY
            QUICKTIME_LIBRARY
            APP_SERVICES_LIBRARY)
    SET(EXTRA_LIBS ${CARBON_LIBRARY} ${QUICKTIME_LIBRARY} ${APP_SERVICES_LIBRARY})
ENDIF (APPLE)

set(WEBRTC_SRC_DIR lib/webrtc/src/)
link_directories(lib/webrtc/src/out/Release/)
file(GLOB libwebrtc lib/webrtc/src/out/Release/*.a)
include_directories(${WEBRTC_SRC_DIR})

find_library(JSONCPP_LIBRARY jsoncpp REQUIRED)


add_definitions("-x objective-c++") # as you already have
set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework AudioToolbox -framework AudioUnit -framework CoreAudio")

# Define shared libraries
set(SHARED_LIBS ${libwebrtc} ${JSONCPP_LIBRARY} boost_thread-mt boost_system-mt)

include(DownloadProject.cmake)
download_project(PROJ                googletest
        GIT_REPOSITORY      https://github.com/google/googletest.git
        GIT_TAG             master
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

set(SOURCE_FILES conductor.cpp conductor.h AbstractDynastat.h Dynastat.cpp Dynastat.h AbstractDynastat.cpp DynastatSimulator.cpp DynastatSimulator.h)
add_executable(cdynastat main.cpp ${SOURCE_FILES})
target_link_libraries(cdynastat ${SHARED_LIBS})

# Test definitions
add_executable(allTests ${SOURCE_FILES} AbstractDynastat_unittest.cpp)
target_link_libraries(allTests gtest_main gtest ${SHARED_LIBS})
