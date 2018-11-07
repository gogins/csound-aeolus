cmake_minimum_required(VERSION 3.5)
project(csound-aeolus)
set(APIVERSION "6.0")
include(ExternalProject)
include_directories(".")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(ALLOW_DUPLICATE_CUSTOM_TARGETS "Yes")
add_definitions(-DUSE_DOUBLE)
set(USE_DOUBLE "Yes")
find_package(Csound)
include_directories("${CSOUND_INCLUDE_DIR}")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
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
    VERSION="1.1.0")
target_include_directories(csound_aeolus PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/aeolus/source" 
    ".")
target_link_libraries(csound_aeolus PRIVATE ${CSOUNDLIB} asound clthreads pthread readline m dl)
set(PLUGIN_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/csound/plugins64-${APIVERSION}")
install(TARGETS csound_aeolus
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIBRARY_INSTALL_DIR})
    
# These are the real targets: the packages.

# distro codename
# find stuff we need
find_program(LSB_RELEASE_CMD lsb_release)
if(NOT DISTRO_CODENAME)
  if(NOT LSB_RELEASE_CMD)
    message(WARNING "DEB Generator: Can't find lsb_release in your path. Setting DISTRO_CODENAME to unknown.")
    set(DISTRO_CODENAME unknown)
  else()
    execute_process(COMMAND ${LSB_RELEASE_CMD} -cs
                    OUTPUT_VARIABLE DISTRO_CODENAME
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
endif()

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Gogins <michael.gogins@gmail.com>")
##set(CPACK_COMPONENTS_ALL csound-base csound-html5 csound-node csound-cmask)
##set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE "OFF")
set(CPACK_GENERATOR TGZ; DEB)
set(CPACK_PACKAGE_CONTACT "${CPACK_DEBIAN_PACKAGE_MAINTAINER}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_NAME "csound-aeolus")
#set(CPACK_PACKAGE_VERSION "$ENV{CSOUND_EXTENDED_VERSION}")
# set(CPACK_STRIP_FILES "YES")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_DEBIAN_COMPONENT_INSTALL "ON")   
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/DEBIAN/triggers")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Extends the sound programming language and library Csound.
 Additional features include Csound opcodes for using the Aeolus software pipe organ 
 by Fons Adriaensen.")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "aeolus, libcsound64-dev")
set(CPACK_DEBIAN_PACKAGE_ENHANCES "csound, csound-python, csoundqt")
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS "ON")
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ">=")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/gogins/csound-aeolus")
set(CPACK_DEBIAN_PACKAGE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS "ON")

# generate a Debian compliant changelog
string(TIMESTAMP RFC2822_TIMESTAMP "%a, %d %b %Y %H:%M:%S -0400" UTC)
set(CHANGELOG_HEADER "csound-extended-dev (${CPACK_PACKAGE_VERSION}) ${DISTRO_CODENAME}\; urgency=high")
# two spaces between maintainer and timestamp is NOT a mistake
set(CHANGELOG_FOOTER " -- ${CPACK_DEBIAN_PACKAGE_MAINTAINER}  ${RFC2822_TIMESTAMP}")
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/git-changelog.sh
    OUTPUT_VARIABLE CHANGELOG 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
# message("CHANGELOG:\n${CHANGELOG}")
string(REPLACE "\"" "" CHANGELOG ${CHANGELOG})
#file(WRITE ${CMAKE_SOURCE_DIR}/debian/changelog.Debian ${CHANGELOG_HEADER}\n${CHANGELOG}\n\n${CHANGELOG_FOOTER})
#execute_process(COMMAND gzip -f -9 -n ${CMAKE_SOURCE_DIR}/debian/changelog.Debian)
file(WRITE ${CMAKE_SOURCE_DIR}/debian/changelog ${CHANGELOG_HEADER}\n${CHANGELOG}\n\n${CHANGELOG_FOOTER})
execute_process(COMMAND gzip -f -9 -n ${CMAKE_SOURCE_DIR}/debian/changelog)
unset(CHANGELOG_HEADER)
unset(CHANGELOG_FOOTER)
unset(RFC2822_TIMESTAMP)

install(FILES "README.md"
    "LICENSE"
    DESTINATION  share/doc/${CPACK_DEBIAN_PACKAGE_NAME})
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME}
    RENAME copyright)
install(FILES ${CMAKE_SOURCE_DIR}/doc/csound_aeolus.pdf
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME})
install(FILES "${CMAKE_SOURCE_DIR}/debian/changelog.gz"
    DESTINATION "share/doc/${CPACK_DEBIAN_PACKAGE_NAME}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples"
    DESTINATION "share/doc/${CPACK_DEBIAN_PACKAGE_NAME}")

include (CPack)






