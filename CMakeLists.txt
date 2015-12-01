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

set(WEBRTC_SRC_DIR /Volumes/Data/Users/ghost/Library/webrtc/src)
link_directories(webrtc/src/osx/Release/)
file(GLOB libwebrtc webrtc/src/osx/Release/*.a)
include_directories(${WEBRTC_SRC_DIR})

add_definitions("-x objective-c++") # as you already have
set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework AudioToolbox -framework AudioUnit -framework CoreAudio")

set(SOURCE_FILES main.cpp)
add_executable(cdynastat ${SOURCE_FILES})
target_link_libraries(cdynastat ${libwebrtc})