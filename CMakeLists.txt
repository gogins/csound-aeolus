cmake_minimum_required(VERSION 3.5)
project(csound-aeolus)
set(APIVERSION "6.0")
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(ALLOW_DUPLICATE_CUSTOM_TARGETS "Yes")
add_definitions(-DUSE_DOUBLE)
set(USE_DOUBLE "Yes")
find_package(Csound)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_PLUGINS_DIR ${BUILD_DIR})
set(BUILD_BIN_DIR ${BUILD_DIR})
set(BUILD_LIB_DIR ${BUILD_DIR})
set(EXECUTABLE_INSTALL_DIR "bin")
option(USE_LIB64 "Set to on to set installation directory for libraries to lib64" OFF)
if(USE_LIB64)
    set(LIBRARY_INSTALL_DIR "lib64")
    add_definitions("-DLIB64")
else()
    set(LIBRARY_INSTALL_DIR "lib")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CPACK_PACKAGING_INSTALL_PREFIX: ${CPACK_PACKAGING_INSTALL_PREFIX}")
message(STATUS "BUILD_BIN_DIR: ${BUILD_BIN_DIR}")
message(STATUS "BUILD_LIB_DIR: ${BUILD_LIB_DIR}")
message(STATUS "BUILD_PLUGINS_DIR: ${BUILD_PLUGINS_DIR}.")
set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins64-${APIVERSION}")
message(STATUS "LIBRARY INSTALL DIR: ${LIBRARY_INSTALL_DIR}")
message(STATUS "PLUGIN_INSTALL_DIR: ${PLUGIN_INSTALL_DIR}")
# Just to make sure it builds and runs on the machine...
ExternalProject_Add(aeolus
    GIT_REPOSITORY  https://salsa.debian.org/multimedia-team/aeolus.git
    GIT_TAG   master
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/aeolus/source/source"
    BUILD_IN_SOURCE "1"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND "")
set(aeolus_csound_SRCS
    src/csound_aeolus.cpp
    src/csound_audio.cc
aeolus/source/source/addsynth.cc
aeolus/source/source/addsynth.h
aeolus/source/source/asection.cc
aeolus/source/source/asection.h
aeolus/source/source/callbacks.h
aeolus/source/source/division.cc
aeolus/source/source/division.h
aeolus/source/source/exp2ap.cc
aeolus/source/source/global.h
aeolus/source/source/iface.h
aeolus/source/source/imidi.cc
aeolus/source/source/imidi.h
aeolus/source/source/lfqueue.cc
aeolus/source/source/lfqueue.h
aeolus/source/source/messages.h
aeolus/source/source/model.cc
aeolus/source/source/model.h
aeolus/source/source/prbsgen.h
aeolus/source/source/rankwave.cc
aeolus/source/source/rankwave.h
aeolus/source/source/reverb.cc
aeolus/source/source/reverb.h
aeolus/source/source/rngen.cc
aeolus/source/source/rngen.h
aeolus/source/source/scales.cc
aeolus/source/source/scales.h
aeolus/source/source/slave.cc
aeolus/source/source/slave.h
aeolus/source/source/tiface.cc
aeolus/source/source/tiface.h
    )
add_library(aeolus_csound SHARED ${aeolus_csound_SRCS})
target_compile_definitions(aeolus_csound PUBLIC 
    VERSION="0.1.0")
target_include_directories(aeolus_csound PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/aeolus/source/source" 
    "/usr/local/include/csound"
    ".")
target_link_libraries(aeolus_csound PRIVATE ${CSOUNDLIB} asound clthreads pthread readline m dl)
set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins64-${APIVERSION}")
install(TARGETS aeolus_csound
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR})







