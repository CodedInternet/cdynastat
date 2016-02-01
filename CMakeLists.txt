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
    SET(BOOST_ROOT /usr/local/opt/boost155)
    SET(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    LINK_DIRECTORIES(/usr/local/opt/boost155/lib)
    MARK_AS_ADVANCED (CARBON_LIBRARY
            QUICKTIME_LIBRARY
            APP_SERVICES_LIBRARY)
    SET(EXTRA_LIBS ${CARBON_LIBRARY} ${QUICKTIME_LIBRARY} ${APP_SERVICES_LIBRARY})
    set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework AudioToolbox -framework AudioUnit -framework CoreAudio")
ENDIF ()

# Dependancy includes
MESSAGE ( STATUS "Searching for X11..." )
FIND_PACKAGE ( X11 REQUIRED )
IF ( X11_FOUND )
    INCLUDE_DIRECTORIES ( ${X11_INCLUDE_DIR} )
    LINK_LIBRARIES ( ${X11_LIBRARIES} )
    MESSAGE ( STATUS " X11_INCLUDE_DIR: " ${X11_INCLUDE_DIR} )
    MESSAGE ( STATUS " X11_LIBRARIES: " ${X11_LIBRARIES} )
ENDIF ( X11_FOUND )

MESSAGE( STATUS "Searching for OpenSSL")
FIND_PACKAGE( OpenSSL REQUIRED )
IF ( OpenSSL_FOUND )
    INCLUDE_DIRECTORIES( ${OPENSSL_INCLUDE_DIR} )
    LINK_LIBRARIES( ${OPENSSL_LIBRARIES} )
    MESSAGE( STATUS " OpenSSL_INCLUDE_DIR: " ${OPENSSL_INCLUDE_DIR} )
    MESSAGE( STATUS " OpenSSL_LIBRARIES: " ${OPENSSL_LIBRARIES} )
ENDIF ( OpenSSL_FOUND )

MESSAGE ( STATUS "Searching for Boost" )
FIND_PACKAGE ( Boost 1.55 REQUIRED COMPONENTS thread system )
IF ( Boost_FOUND )
    INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
    LINK_LIBRARIES( ${Boost_LIBRARIES} )
    MESSAGE( STATUS " Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR} )
    MESSAGE( STATUS " Boost_LIBRARIES: " ${Boost_LIBRARIES} )
ENDIF ( Boost_FOUND )



# Project includes
link_directories(lib)
include_directories(include)

IF (NOT DEFINED ARCH)
    SET(ARCH "amd64")
ENDIF ()

download_project(PROJ websocketpp
        GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
        GIT_TAG 0.6.0
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

add_subdirectory(${websocketpp_SOURCE_DIR} ${websocketpp_BINARY_DIR} EXCLUDE_FROM_ALL)
include_directories(${websocketpp_SOURCE_DIR})

set(LIBWEBRTC libwebrtc-${CMAKE_SYSTEM_NAME}-${ARCH}-r49.a)

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
#if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
#endif()

# Main executable
SET(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)

# Test definitions
add_subdirectory(test)
