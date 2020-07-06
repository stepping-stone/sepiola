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

if(DEFINED RESELLER)
    message("Creating makefile for a reseller")
    set(IS_RESELLER TRUE FORCE)
endif(DEFINED RESELLER)

set(MACOSX_ICON_FILE
    src/res/mac/icon.icns
    CACHE PATH "Application icon for OSX")
set(WINDOWS_RESSOURCE_FILE
    src/res/windows/resource.rc
    CACHE PATH "Windows ressource file")
set(LINUX_ICON_FILE
    src/res/linux/icon.png
    CACHE PATH "Application icon for Linux")
set(ABOUT_IMAGE_FILE
    src/gui/resources/about_image.png
    CACHE PATH "About image")
set(WINDOW_ICON_FILE
    src/gui/resources/icon.png
    CACHE PATH "Window icon")
set(RESELLER_CONFIG_FILE
    config/config_reseller
    CACHE PATH "Configuration file for the reseller")

if(CMAKE_BUILD_TYPE STREQUAL Deploy)
    add_definitions(-DCMAKE_BUILD_TYPE_DEPLOY)
endif(CMAKE_BUILD_TYPE STREQUAL Deploy)

if(CMAKE_COMPILER_IS_GNUCXX)
    # The GCC on Travis CI does not support -std=c++11 yet
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuninitialized -Winit-self")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wstrict-null-sentinel")
    # class members are always accessed via this, shadowing members is therefore allowed
    #    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal -Wno-long-long")
else(CMAKE_COMPILER_IS_GNUCXX)
    # on Clang we have one flag to rule them all: -Weverything (includes all
    # warnings, including the ones enabled with -pedantic)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Weverything")
    # disable warnings about variables being shadowed, see above (this is just Qt-style)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow")
    # disable warnings about padding member attributes since there is nothing
    # we can do about it at the moment
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")
    # disable warnings about C++98 incompatibility since we require C++11
    # from now on anyway
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
endif(CMAKE_COMPILER_IS_GNUCXX)

# The following works on GNU C++ and LLVM only, but those are the only ones we
# currently support anyway (C++ native, MinGW C++ for win32 and LLVM on OSX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -ggdb") # -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_CXX_FLAGS_DEPLOY ${CMAKE_CXX_FLAGS_RELWITHDEBUGINFO})

if(Q_WS_WIN)
    include(CMakeDetermineRCCompiler)
    # add windows icon/ressources
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.o
        COMMAND ${CMAKE_RC_COMPILER} ${PROJECT_SOURCE_DIR}/${WINDOWS_RESSOURCE_FILE} -o
                ${CMAKE_CURRENT_BINARY_DIR}/resource.o)
    set(sepiola_RC ${sepiola_RC} ${CMAKE_CURRENT_BINARY_DIR}/resource.o)
endif(Q_WS_WIN)

generate_qt_resource_from_files(
    "${CMAKE_CURRENT_BINARY_DIR}/logos_images.qrc" "/main"
    "${PROJECT_SOURCE_DIR}/${ABOUT_IMAGE_FILE};${PROJECT_SOURCE_DIR}/${WINDOW_ICON_FILE}")

add_definitions(${QT_DEFINITIONS})

if(Q_WS_WIN)
    set(OS_HEADER_DIRECTORIES /usr/local/inc/win2003/)
endif(Q_WS_WIN)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR} ${OS_HEADER_DIRECTORIES})

include_directories(SYSTEM ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
                    ${QT_QTNETWORK_INCLUDE_DIR} ${OS_HEADER_DIRECTORIES})

find_program(
    QT_LUPDATE
    NAMES lupdate-qt4 lupdate
    PATHS $ENV{QTDIR}/bin
    DOC "Path to the lupdate program")

find_program(
    QT_LRELEASE
    NAMES lrelease-qt4 lrelease
    PATHS $ENV{QTDIR}/bin
    DOC "Path to the lrelease program")

set(translation_sources ${translation_sources} i18n/app_de.ts)
set(qt_translations /usr/share/qt4/translations/qt_de.qm)

# Update translations (note that lupdate target write to source directory)
if(QT_LUPDATE)
    # Update all translations and remove obsolete strings
    add_custom_target(
        lupdate
        ${QT_LUPDATE} -noobsolete ${translate_files} -ts ${translation_sources}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # Create template.ts to be used for new translations
    add_custom_command(
        OUTPUT template.ts
        COMMAND ${QT_LUPDATE} -noobsolete ${translate_files} -ts template.ts
        DEPENDS ${translate_files})
    add_custom_target(create-ts-template DEPENDS template.ts)
else(QT_LUPDATE)
    message(STATUS "lupdate could not be found. You will not be able to update the translations.")
endif(QT_LUPDATE)

# Build translations
if(QT_LRELEASE)
    # Clear list
    set(qms)

    # Add command to build X.qm from X.ts
    foreach(ts ${translation_sources})
        get_filename_component(qm ${ts} NAME_WE)
        set(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
        set(ts "${CMAKE_CURRENT_SOURCE_DIR}/${ts}")
        add_custom_command(
            OUTPUT ${qm}
            COMMAND ${QT_LRELEASE} ${ts} -qm ${qm}
            DEPENDS ${ts})
        set(qms ${qms} ${qm})
        set(translation_binaries ${translation_binaries} ${qm})
    endforeach(ts)

    # Finish off with a command that builds *.qm
    add_custom_target(lrelease ALL DEPENDS ${qms})
else(QT_LRELEASE)
    message(STATUS "lrelease could not be found. You're losing the localisation.")
endif(QT_LRELEASE)

# Clear list
set(qms)

foreach(qt_translation ${qt_translations})
    get_filename_component(qm ${qt_translation} NAME_WE)
    set(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
    add_custom_command(
        OUTPUT ${qm}
        COMMAND ${CMAKE_COMMAND} -E copy ${qt_translation} ${qm}
        DEPENDS ${qt_translation})
    set(qms ${qms} ${qm})
    set(translation_binaries ${translation_binaries} ${qm})
endforeach(qt_translation)

# Finish off with a command that builds *.qm
add_custom_target(copy-qt-translation ALL DEPENDS ${qms})

set(PORTABLE_INSTALLATION
    TRUE
    CACHE BOOL "Build a portable version (ignore installation prefix)")

if(NOT PORTABLE_INSTALLATION)
    set(DIRECTORY_SHARE
        "${CMAKE_INSTALL_PREFIX}/share"
        CACHE PATH "Set the share directory")
    set(APPLICATION_SHARE_DIR
        "${DIRECTORY_SHARE}/${SSBACKUP_EXECUTABLE_NAME}"
        CACHE INTERNAL "Set the share directory" FORCE)
endif(NOT PORTABLE_INSTALLATION)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

qt4_wrap_cpp(
    sepiola_base_MOC
    tools/abstract_informing_process.hh
    tools/abstract_rsync.hh
    tools/abstract_scheduler.hh
    tools/abstract_snapshot.hh
    tools/dummy_snapshot.hh
    tools/filesystem_snapshot.hh
    tools/rsync.hh
    settings/settings.hh
    utils/log_file_utils.hh
    utils/single_application_guard.hh
    model/backup_thread.hh
    model/main_model.hh
    model/restore_thread.hh
    model/space_usage_model.hh)

qt4_wrap_cpp(
    sepiola_ui_MOC
    cli/cli_manager.hh
    gui/about_dialog.hh
    gui/backup_form.hh
    gui/logfile_form.hh
    gui/main_window.hh
    gui/overview_form.hh
    gui/restore_form.hh
    gui/settings_form.hh
    gui/text_input_dialog.hh
    gui/password_dialog.hh
    gui/pattern_dialog.hh
    gui/output_dialog.hh
    gui/traffic_progress_dialog.hh
    gui/stacked_bar_view.hh)

qt4_wrap_ui(
    sepiola_UI
    gui/about_dialog.ui
    gui/backup_form.ui
    gui/logfile_form.ui
    gui/main_window.ui
    gui/overview_form.ui
    gui/restore_form.ui
    gui/settings_form.ui
    gui/text_input_dialog.ui
    gui/password_dialog.ui
    gui/pattern_dialog.ui
    gui/output_dialog.ui
    gui/traffic_progress_dialog.ui)

qt4_add_resources(sepiola_RC gui/resources/screen.qrc
                  "${CMAKE_CURRENT_BINARY_DIR}/logos_images.qrc")

set(BASE_SOURCE_FILES
    settings/settings.cc
    tools/plink.cc
    tools/process.cc
    tools/rsync.cc
    tools/dummy_snapshot.cc
    tools/filesystem_snapshot.cc
    tools/filesystem_snapshot_path_mapper.cc
    tools/tool_factory.cc
    utils/string_utils.cc
    utils/extended_file.cc
    utils/file_system_utils.cc
    utils/log_file_utils.cc
    utils/host_file_utils.cc
    utils/progress_task.cc
    utils/single_application_guard.cc
    model/backup_thread.cc
    model/dir_tree_item.cc
    model/main_model.cc
    model/restore_name.cc
    model/restore_thread.cc
    model/remote_dir_model.cc
    model/local_dir_model.cc
    model/scheduled_task.cc
    model/backup_task.cc
    model/space_usage_model.cc
    ${sepiola_base_MOC})

set(UI_SOURCE_FILES
    main.cc
    cli/cli_manager.cc
    gui/about_dialog.cc
    gui/backup_form.cc
    gui/logfile_form.cc
    gui/main_window.cc
    gui/overview_form.cc
    gui/traffic_progress_dialog.cc
    gui/output_dialog.cc
    gui/password_dialog.cc
    gui/pattern_dialog.cc
    gui/restore_form.cc
    gui/settings_form.cc
    gui/text_input_dialog.cc
    gui/stacked_bar_view.cc
    test/test_manager.cc
    ${sepiola_UI}
    ${sepiola_ui_MOC}
    ${sepiola_CFG})

# Use Q_WS_WIN from Qt-module instead of WIN32 from cmake
# to make it work when cross-compiling using MinGW
if(Q_WS_WIN)
    qt4_wrap_cpp(sepiola_win_MOC tools/shadow_copy.hh tools/optionalSnapshot.hh)
    list(
        APPEND
        BASE_SOURCE_FILES
        tools/at.cc
        tools/schtasks.cc
        tools/set_acl.cc
        tools/shadow_copy.cc
        tools/optionalSnapshot.cc
        ${sepiola_win_MOC})
elseif(Q_WS_MAC)
    list(APPEND BASE_SOURCE_FILES tools/unix_permissions.cc)
elseif(Q_WS_X11)
    list(APPEND BASE_SOURCE_FILES tools/crontab.cc tools/posix_acl.cc)
endif(Q_WS_WIN)

add_library(sepiola_base_objects OBJECT ${BASE_SOURCE_FILES})
add_library(sepiola_ui_objects OBJECT ${UI_SOURCE_FILES})

add_executable(
    ${SSBACKUP_EXECUTABLE_NAME} WIN32 MACOSX_BUNDLE
    $<TARGET_OBJECTS:sepiola_base_objects> $<TARGET_OBJECTS:sepiola_ui_objects> ${sepiola_RC})

if(Q_WS_WIN)
    add_executable(
        ${SSBACKUP_EXECUTABLE_NAME}.com $<TARGET_OBJECTS:sepiola_base_objects>
                                        $<TARGET_OBJECTS:sepiola_ui_objects> ${sepiola_RC})
    set_target_properties(${SSBACKUP_EXECUTABLE_NAME}.com PROPERTIES SUFFIX ""
    )# don't add .exe suffix

    # for second binary
    target_link_libraries(${SSBACKUP_EXECUTABLE_NAME}.com ${QT_QTGUI_LIBRARY}
                          ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY})
    target_link_libraries(${SSBACKUP_EXECUTABLE_NAME}.com debug ${QT_QTGUI_LIBRARY_DEBUG}
                          ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTCORE_LIBRARY_DEBUG})
endif(Q_WS_WIN)

target_link_libraries(${SSBACKUP_EXECUTABLE_NAME} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
                      ${QT_QTCORE_LIBRARY})
target_link_libraries(${SSBACKUP_EXECUTABLE_NAME} debug ${QT_QTGUI_LIBRARY_DEBUG}
                      ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTCORE_LIBRARY_DEBUG})

if(CMAKE_BUILD_TYPE STREQUAL Deploy)
    if(Q_WS_WIN)
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}
            ${QT_GUI_LIB_DEPENDENCIES}
            ${QT_NETWORK_LIB_DEPENDENCIES}
            ${QT_CORE_LIB_DEPENDENCIES}
            ${QT_PLUGINS_DIR}/imageformats/libqsvg.a
            ${QT_QTSVG_LIBRARY}
            ${QT_QTXML_LIBRARY})
        target_link_libraries(${SSBACKUP_EXECUTABLE_NAME} ole32 oleaut32 uuid winspool)
        # for second binary
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}.com
            ${QT_GUI_LIB_DEPENDENCIES}
            ${QT_NETWORK_LIB_DEPENDENCIES}
            ${QT_CORE_LIB_DEPENDENCIES}
            ${QT_PLUGINS_DIR}/imageformats/libqsvg.a
            ${QT_QTSVG_LIBRARY}
            ${QT_QTXML_LIBRARY})
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}.com
            ole32
            oleaut32
            uuid
            winspool
            gdi32
            comdlg32)
    elseif(Q_WS_MAC)
        set(CMAKE_CXX_FLAGS_DEPLOY
            "${CMAKE_CXX_FLAGS_DEPLOY} -mmacosx-version-min=10.3 -arch ppc -arch i386")
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}
            ${QT_GUI_LIB_DEPENDENCIES}
            ${QT_NETWORK_LIB_DEPENDENCIES}
            ${QT_CORE_LIB_DEPENDENCIES}
            ${QT_PLUGINS_DIR}/imageformats/libqsvg.a
            ${QT_QTSVG_LIBRARY}
            ${QT_QTXML_LIBRARY})
        target_link_libraries(${SSBACKUP_EXECUTABLE_NAME} "-framework AppKit" iconv System)
    else(Q_WS_WIN)
        set(CMAKE_CXX_LINK_EXECUTABLE
            "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        )
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}
            -static-libgcc
            -static-libstdc++
            -Wl,-Bstatic
            Xfixes
            Xrender
            Xrandr
            Xcursor
            Xinerama
            freetype
            expat
            stdc++
            -Wl,-Bdynamic)
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME} ${QT_NETWORK_LIB_DEPENDENCIES} ${QT_CORE_LIB_DEPENDENCIES}
            ${QT_PLUGINS_DIR}/imageformats/libqsvg.a ${QT_QTSVG_LIBRARY} ${QT_QTXML_LIBRARY})
        target_link_libraries(
            ${SSBACKUP_EXECUTABLE_NAME}
            SM
            ICE
            Xi
            Xext
            X11
            rt
            m
            fontconfig)
    endif(Q_WS_WIN)
endif(CMAKE_BUILD_TYPE STREQUAL Deploy)

set(APP_DIR bin)
set(RESOURCE_DIR resource)
if(Q_WS_MAC)
    set(APP_DIR ${PROJECT_NAME})
endif(Q_WS_MAC)

install(TARGETS ${SSBACKUP_EXECUTABLE_NAME} RUNTIME DESTINATION ${APP_DIR})
# for second binary
if(Q_WS_WIN)
    install(TARGETS ${SSBACKUP_EXECUTABLE_NAME}.com RUNTIME DESTINATION ${APP_DIR})
endif(Q_WS_WIN)

if(Q_WS_WIN)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WIN_DIR windows-x86_64)
    else()
        set(WIN_DIR windows-i386)
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    file(GLOB PLINK_BIN ${PROJECT_SOURCE_DIR}/tools/${WIN_DIR}/plink-*)
    file(GLOB SETACL_BIN ${PROJECT_SOURCE_DIR}/tools/windows-i386/setacl-*)
    add_custom_target(
        copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/config/config
                ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PLINK_BIN}
                ${CMAKE_CURRENT_BINARY_DIR}/plink.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SETACL_BIN}
                ${CMAKE_CURRENT_BINARY_DIR}/setacl.exe)

    install(FILES ${PROJECT_SOURCE_DIR}/config/config DESTINATION ${APP_DIR})
    if(DEFINED RESELLER)
        install(FILES ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE} DESTINATION ${APP_DIR})
        add_custom_target(
            copy_reseller ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR})
    endif(DEFINED RESELLER)
    install(FILES ${translation_binaries} DESTINATION ${APP_DIR})

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/plink.exe ${CMAKE_CURRENT_BINARY_DIR}/setacl.exe
            DESTINATION ${APP_DIR})
elseif(Q_WS_MAC)
    add_custom_target(
        copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/config/config
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/plink-*
            ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink
        COMMAND chmod u+x
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/ssh-*
            ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh
        COMMAND chmod u+x
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/rsync-*
            ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync
        COMMAND chmod u+x
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${translation_binaries}
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/${MACOSX_ICON_FILE}
            ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/Resources/icon.icns
    )

    # work around cmake bug
    if(NOT CMAKE_BUILD_TYPE STREQUAL Deploy)
        set_target_properties(${SSBACKUP_EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-framework QtGui")
    endif(NOT CMAKE_BUILD_TYPE STREQUAL Deploy)

    install(FILES ${PROJECT_SOURCE_DIR}/config/config
            DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
    if(DEFINED RESELLER)
        install(FILES ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE}
                DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
        add_custom_target(
            copy_reseller ALL
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE}
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
    endif(DEFINED RESELLER)

    install(FILES ${translation_binaries}
            DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink
        DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh
            DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync
        DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
else(Q_WS_WIN)
    execute_process(
        COMMAND arch
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(ARCH MATCHES "64$")
        set(LINUX_DIR linux-x86_64)
    else(ARCH MATCHES "64$")
        set(LINUX_DIR linux-i386)
    endif(ARCH MATCHES "64$")
    add_custom_target(
        copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/config/config
                ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/plink-* ${CMAKE_CURRENT_BINARY_DIR}/plink
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/plink
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/ssh-*
                ${CMAKE_CURRENT_BINARY_DIR}/ssh
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/ssh
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/libcrypto.so.1.1.1c
            ${CMAKE_CURRENT_BINARY_DIR}/libcrypto.so.1.1.1c
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/libcrypto.so.1.1.1c
        COMMAND ${CMAKE_COMMAND} -E create_symlink libcrypto.so.1.1.1c
                ${CMAKE_CURRENT_BINARY_DIR}/libcrypto.so.1.1
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/rsync-* ${CMAKE_CURRENT_BINARY_DIR}/rsync
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/rsync
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/getfacl-*
            ${CMAKE_CURRENT_BINARY_DIR}/getfacl
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/getfacl
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/setfacl-*
            ${CMAKE_CURRENT_BINARY_DIR}/setfacl
        COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/setfacl
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/${LINUX_ICON_FILE}
                ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.png)

    install(FILES ${PROJECT_SOURCE_DIR}/config/config DESTINATION ${APP_DIR})
    if(DEFINED RESELLER)
        install(FILES ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE} DESTINATION ${APP_DIR})
        add_custom_target(
            copy_reseller ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${PROJECT_SOURCE_DIR}/${RESELLER_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM)
    endif(DEFINED RESELLER)
    install(FILES ${translation_binaries} DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/plink DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ssh DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libcrypto.so.1.1.1c DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libcrypto.so.1.1 DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rsync DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/getfacl DESTINATION ${APP_DIR})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/setfacl DESTINATION ${APP_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.png
            DESTINATION ${RESOURCE_DIR})
endif(Q_WS_WIN)

add_custom_target(copyright)

add_subdirectory(test)
