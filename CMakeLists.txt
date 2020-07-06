#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

cmake_minimum_required(VERSION 2.8.11)

# If available, enable developer warnings for the build system
if(POLICY CMP0005)
    cmake_policy(SET CMP0005 NEW)
endif(POLICY CMP0005)

if(NOT SSBACKUP_EXECUTABLE_NAME)
    set(SSBACKUP_EXECUTABLE_NAME sepiola)
endif(NOT SSBACKUP_EXECUTABLE_NAME)

if(NOT SSBACKUP_APPLICATION_FULL_NAME)
    set(SSBACKUP_APPLICATION_FULL_NAME "Sepiola Online Backup Client")
endif(NOT SSBACKUP_APPLICATION_FULL_NAME)

project(${SSBACKUP_EXECUTABLE_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(Macros)
include(TargetArch)
include(GetGitRevisionDescription)

# enable CTest module
enable_testing()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(Qt4_FIND_REQUIRED TRUE)
include(FindQt4)

# Translatable source files
file(GLOB_RECURSE translate_files src/*.cc src/*.hh src/*.ui)

git_describe(GIT_DESC --tags --dirty)

if(GIT_DESC)
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" CPACK_PACKAGE_VERSION_MAJOR "${GIT_DESC}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" CPACK_PACKAGE_VERSION_MINOR "${GIT_DESC}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" CPACK_PACKAGE_VERSION_PATCH
                         "${GIT_DESC}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_GIT "${GIT_DESC}")

    execute_process(
        COMMAND git log -1 --format=%ai
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    # take only the date from the git timestamp:
    string(REGEX REPLACE "^([0-9\\-]+) .*" "\\1" SSBACKUP_RELEASE_DATE "${GIT_COMMIT_DATE}")
else()
    set(CPACK_PACKAGE_VERSION_MAJOR 2)
    set(CPACK_PACKAGE_VERSION_MINOR 6)
    set(CPACK_PACKAGE_VERSION_PATCH 1)
    set(SSBACKUP_RELEASE_DATE "2020-07-06")
endif()

# set automatically by CMake, but we need it early
set(CPACK_PACKAGE_VERSION
    "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${VERSION_GIT}"
)

set(CPACK_PACKAGE_NAME ${SSBACKUP_EXECUTABLE_NAME})
set(CPACK_PACKAGE_FULL_NAME ${SSBACKUP_APPLICATION_FULL_NAME})
set(CPACK_PACKAGE_VENDOR "stepping stone AG")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_FULL_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_NSIS_QT_SUFFIX "d")
    set(CPACK_PACKAGE_FILE_NAME_SUFFIX "-debug")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Has to be below the variables because they are accessed in the src/CMakeLists.txt
add_subdirectory(src)

set(CPACK_SSBACKUP_EXECUTABLE_NAME "${SSBACKUP_EXECUTABLE_NAME}")
set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CPACK_SSBACKUP_APPLICATION_FULL_NAME "${SSBACKUP_APPLICATION_FULL_NAME}")

target_architecture(TARGET_ARCH)

if(Q_WS_WIN)
    set(CPACK_GENERATOR "NSIS")
    if(TARGET_ARCH STREQUAL "x86_64")
        set(WINDOWS_NAME win64)
        set(CPACK_PROGRAMFILES_NAME "PROGRAMFILES64")
    else()
        set(WINDOWS_NAME win32)
        set(CPACK_PROGRAMFILES_NAME "PROGRAMFILES32")
    endif()
    set(CPACK_PACKAGE_FILE_NAME
        ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${WINDOWS_NAME}${CPACK_PACKAGE_FILE_NAME_SUFFIX})
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer/windows/" ${CMAKE_MODULE_PATH})
    include(CPack)
elseif(Q_WS_MAC)
    add_custom_target(
        package
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/staging
        COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/staging -P
                cmake_install.cmake
        COMMAND
            hdiutil create -ov -srcfolder ${CMAKE_BINARY_DIR}/staging/${PROJECT_NAME}
            ${SSBACKUP_EXECUTABLE_NAME}-${CPACK_PACKAGE_VERSION}-Mac-universal${CPACK_PACKAGE_FILE_NAME_SUFFIX}.dmg
    )
else(Q_WS_WIN)
    execute_process(COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CPACK_GENERATOR "STGZ")
    set(CPACK_PACKAGE_FILE_NAME
        ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-Linux-${TARGET_ARCH}${CPACK_PACKAGE_FILE_NAME_SUFFIX}
    )
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer/linux/" ${CMAKE_MODULE_PATH})
    include(CPack)
endif(Q_WS_WIN)
