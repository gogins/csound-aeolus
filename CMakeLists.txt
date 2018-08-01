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
add_library(csound_aeolus SHARED 
    src/csound_aeolus.cpp
    src/audio.cc
    src/audio.h
    src/tiface.cc
    src/tiface.h
    aeolus/source/addsynth.cc
    aeolus/source/addsynth.h
    aeolus/source/asection.cc
    aeolus/source/asection.h
    aeolus/source/callbacks.h
    aeolus/source/division.cc
    aeolus/source/division.h
    aeolus/source/exp2ap.cc
    aeolus/source/global.h
    aeolus/source/iface.h
    aeolus/source/imidi.cc
    aeolus/source/imidi.h
    aeolus/source/lfqueue.cc
    aeolus/source/lfqueue.h
    aeolus/source/messages.h
    aeolus/source/model.cc
    aeolus/source/model.h
    aeolus/source/prbsgen.h
    aeolus/source/rankwave.cc
    aeolus/source/rankwave.h
    aeolus/source/reverb.cc
    aeolus/source/reverb.h
    aeolus/source/rngen.cc
    aeolus/source/rngen.h
    aeolus/source/scales.cc
    aeolus/source/scales.h
    aeolus/source/slave.cc
    aeolus/source/slave.h
    )
target_compile_definitions(csound_aeolus PUBLIC 
    VERSION="0.1.0")
target_include_directories(csound_aeolus PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/aeolus/source" 
    "/usr/local/include/csound"
    ".")
target_link_libraries(csound_aeolus PRIVATE ${CSOUNDLIB} asound clthreads pthread readline m dl)
set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins64-${APIVERSION}")
install(TARGETS csound_aeolus
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR})
set(SHARE_INSTALL_DIR "share/csound-aeolus")
install(FILES "README.md"
    "LICENSE"
    DESTINATION  ${SHARE_INSTALL_DIR})
install(FILES "doc/csound_aeolus.pdf"
    DESTINATION  ${SHARE_INSTALL_DIR})

include (CPack)






