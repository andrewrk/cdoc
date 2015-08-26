cmake_minimum_required(VERSION 2.8.5)

if(CMAKE_VERSION VERSION_LESS 3.0.0)
    set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "library install dir (lib)")
    set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "header base install dir (include)")
    set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
else()
    cmake_policy(SET CMP0042 NEW)
    cmake_policy(SET CMP0046 NEW)
    include(GNUInstallDirs)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(cdoc C)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CDOC_VERSION_MAJOR 0)
set(CDOC_VERSION_MINOR 0)
set(CDOC_VERSION_PATCH 0)
set(CDOC_VERSION "${CDOC_VERSION_MAJOR}.${CDOC_VERSION_MINOR}.${CDOC_VERSION_PATCH}")
message("Configuring cdoc version ${CDOC_VERSION}")

find_package(clang)
include_directories(${CLANG_INCLUDE_DIR})

set(CDOC_SOURCES
    "${CMAKE_SOURCE_DIR}/src/main.c"
)

set(CONFIGURE_OUT_FILE "${CMAKE_BINARY_DIR}/config.h")


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Werror -pedantic")


set(EXE_CFLAGS "-std=c11 -fvisibility=hidden -Wall -Werror=strict-prototypes -Werror=old-style-definition -Werror=missing-prototypes")

configure_file (
    "${CMAKE_SOURCE_DIR}/src/config.h.in"
    ${CONFIGURE_OUT_FILE}
)

add_executable(cdoc ${CDOC_SOURCES})
set_target_properties(cdoc PROPERTIES
    LINKER_LANGUAGE C
    COMPILE_FLAGS ${EXE_CFLAGS})
target_link_libraries(cdoc LINK_PUBLIC
    ${CLANG_LIBRARY}
)
install(TARGETS cdoc DESTINATION ${CMAKE_INSTALL_BINDIR})

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
