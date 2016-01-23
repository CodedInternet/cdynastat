cmake_minimum_required(VERSION 2.8.2)
project(cdynastat)
include(DownloadProject.cmake)
# https://github.com/Crascit/DownloadProject/blob/master/CMakeLists.txt
if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else ()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif ()

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
    set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework AudioToolbox -framework AudioUnit -framework CoreAudio")
ENDIF (APPLE)

link_directories(lib)
include_directories(include)

download_project(PROJ jsoncpp
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG 0.10.5
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR} EXCLUDE_FROM_ALL)

include_directories("${jsoncpp_SOURCE_DIR}/include")

IF (NOT ARCH)
    SET(ARCH "amd64")
ENDIF (LINUX)

SET(WEBRTC_NAMES libwebrtc-${CMAKE_SYSTEM_NAME}-${ARCH}-r49.a)

# Define shared libraries
set(SHARED_LIBS ${WEBRTC_NAMES} jsoncpp boost_thread-mt boost_system-mt)

# gtest DownloadProject

download_project(PROJ googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG master
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
endif()

set(SOURCE_FILES conductor.cpp conductor.h AbstractDynastat.h Dynastat.cpp Dynastat.h AbstractDynastat.cpp DynastatSimulator.cpp DynastatSimulator.h)
add_executable(cdynastat main.cpp ${SOURCE_FILES})
target_link_libraries(cdynastat ${SHARED_LIBS})

# Test definitions
add_executable(allTests ${SOURCE_FILES} AbstractDynastat_unittest.cpp)
target_link_libraries(allTests gtest_main gtest ${SHARED_LIBS})
