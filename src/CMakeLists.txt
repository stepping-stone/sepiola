#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
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

IF(DEFINED RESELLER)
	IF(${RESELLER} STREQUAL ${STEPPINGSTONE_RESELLER_NAME})
		ADD_DEFINITIONS(-DCPACK_IS_RESELLER=0)
	ELSE(${RESELLER} STREQUAL ${STEPPINGSTONE_RESELLER_NAME})
		MESSAGE("Creating makefile for a reseller")
		ADD_DEFINITIONS(-DCPACK_IS_RESELLER=1)
	ENDIF(${RESELLER} STREQUAL ${STEPPINGSTONE_RESELLER_NAME})
ELSE(DEFINED RESELLER)
	ADD_DEFINITIONS(-DCPACK_IS_RESELLER=0)
ENDIF(DEFINED RESELLER)

SET(MACOSX_BUNDLE_ICON_FILE icon.icns)
SET(WIN_BUNDLE_ICON_FILE    icon.ico)
SET(LINUX_BUNDLE_ICON_FILE  icon.png)


ADD_DEFINITIONS(-DCPACK_PACKAGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR})
ADD_DEFINITIONS(-DCPACK_PACKAGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR})
ADD_DEFINITIONS(-DCPACK_PACKAGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH})
ADD_DEFINITIONS(-DSSBACKUP_EXECUTABLE_NAME=\\"${SSBACKUP_EXECUTABLE_NAME}\\")

IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -Wall")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wuninitialized -Winit-self")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
SET(CMAKE_CXX_FLAGS_DEPLOY ${CMAKE_CXX_FLAGS_RELWITHDEBUGINFO})

IF(Q_WS_WIN)
	# add windows icon
	SET(ICON_RC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/icon/windows)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
		COMMAND windres.exe -I${ICON_RC_DIR}
		-i${CMAKE_CURRENT_SOURCE_DIR}/icon/windows/icon.rc
		-o ${CMAKE_CURRENT_BINARY_DIR}/icon.o
	)
	SET(sepiola_RC ${sepiola_RC} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
ENDIF(Q_WS_WIN)

# add reseller logo: copy reseller logo and appropriate qrc-file into build-dir
SET(RESELLER_LOGO_QRC_FULLFILENAME "${CMAKE_CURRENT_BINARY_DIR}/resources/logo_reseller.qrc")
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/src/gui/resources/logo_reseller.qrc" ${RESELLER_LOGO_QRC_FULLFILENAME} COPYONLY)
ADD_CUSTOM_COMMAND(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/resources/logo_reseller.png"
	DEPENDS "${PROJECT_SOURCE_DIR}/src/gui/resources/logo_reseller.png"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/src/gui/resources/logo_reseller.png" "${CMAKE_CURRENT_BINARY_DIR}/resources/logo_reseller.png"
)


ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(
	.
	${QT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR} 
	${QT_QTGUI_INCLUDE_DIR}
	${QT_QTNETWORK_INCLUDE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
)

FIND_PROGRAM(QT_LUPDATE 
	NAMES lupdate-qt4 lupdate
	PATHS $ENV{QTDIR}/bin
	DOC "Path to the lupdate program"
)

FIND_PROGRAM(QT_LRELEASE
	NAMES lrelease-qt4 lrelease
	PATHS $ENV{QTDIR}/bin
	DOC "Path to the lrelease program"
)

SET(translation_sources ${translation_sources} i18n/German.ts)

# Update translations (note that lupdate target write to source directory)
IF(QT_LUPDATE)
	# Update all translations and remove obsolete strings
	ADD_CUSTOM_TARGET(
		lupdate
		${QT_LUPDATE} -noobsolete ${translate_files} -ts ${translation_sources}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	# Create template.ts to be used for new translations
	ADD_CUSTOM_COMMAND(OUTPUT template.ts
		COMMAND ${QT_LUPDATE} -noobsolete ${translate_files} -ts template.ts
		DEPENDS ${translate_files}
	)
	ADD_CUSTOM_TARGET(create-ts-template DEPENDS template.ts)
ELSE(QT_LUPDATE)
	MESSAGE(STATUS "lupdate could not be found. You will not be able to update the translations.")
ENDIF(QT_LUPDATE)

# Build translations
IF(QT_LRELEASE)
	# Clear list
	SET(qms)

	# Add command to build X.qm from X.po
	FOREACH(po ${translation_sources})
		GET_FILENAME_COMPONENT(qm ${po} NAME_WE)
		SET(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
		SET(po "${CMAKE_CURRENT_SOURCE_DIR}/${po}")
		ADD_CUSTOM_COMMAND(OUTPUT ${qm} 
		COMMAND ${QT_LRELEASE} ${po} -qm ${qm}
		DEPENDS ${po})
		SET(qms ${qms} ${qm})
		SET(translation_binaries ${translation_binaries} ${qm})
	ENDFOREACH(po)

	# Finish off with a command that builds *.qm
	ADD_CUSTOM_TARGET(lrelease ALL DEPENDS ${qms})
ELSE(QT_LRELEASE)
	MESSAGE(STATUS "lrelease could not be found. You're losing the localisation.")
ENDIF(QT_LRELEASE)


QT4_WRAP_CPP(
	sepiola_MOC 
	cli/cli_manager.hh
	gui/about_dialog.hh
	gui/backup_form.hh 
	gui/logfile_form.hh
	gui/main_window.hh 
	gui/restore_form.hh 
	gui/schedule_dialog.hh
	gui/settings_form.hh
	gui/text_input_dialog.hh
	gui/password_dialog.hh 
	gui/pattern_dialog.hh
	gui/output_dialog.hh 
	model/backup_thread.hh
	model/main_model.hh
	model/restore_thread.hh
	tools/abstract_progress.hh
	tools/abstract_rsync.hh
	tools/abstract_scheduler.hh
	tools/rsync.hh
	settings/settings.hh
	utils/log_file_utils.hh
)

QT4_WRAP_UI(
	sepiola_UI 
	gui/about_dialog.ui 
	gui/backup_form.ui 
	gui/logfile_form.ui
	gui/main_window.ui 
	gui/restore_form.ui 
	gui/schedule_dialog.ui
	gui/settings_form.ui 
	gui/text_input_dialog.ui
	gui/password_dialog.ui 
	gui/pattern_dialog.ui
	gui/output_dialog.ui
)

QT4_ADD_RESOURCES(
	sepiola_RC
	gui/resources/screen.qrc
	${RESELLER_LOGO_QRC_FULLFILENAME}
)

SET(SOURCE_FILES
	main.cc 
	cli/cli_manager.cc
	gui/about_dialog.cc
	gui/backup_form.cc 
	gui/logfile_form.cc
	gui/main_window.cc 
	gui/output_dialog.cc
	gui/password_dialog.cc
	gui/pattern_dialog.cc
	gui/restore_form.cc 
	gui/schedule_dialog.cc
	gui/settings_form.cc
	gui/text_input_dialog.cc
	model/backup_thread.cc
	model/dir_tree_item.cc
	model/main_model.cc 
	model/restore_name.cc
	model/restore_thread.cc
	model/remote_dir_model.cc
	settings/settings.cc
	test/test_manager.cc
	tools/at.cc 
	tools/schtasks.cc
	tools/crontab.cc
	tools/plink.cc 
	tools/posix_acl.cc 
	tools/process.cc 
	tools/rsync.cc 
	tools/set_acl.cc
	tools/unix_permissions.cc
	tools/tool_factory.cc 
	utils/extended_file.cc
	utils/file_system_utils.cc
	utils/log_file_utils.cc
	utils/host_file_utils.cc
	${sepiola_UI} 
	${sepiola_MOC} 
	${sepiola_RC}
	${sepiola_CFG}
)

ADD_EXECUTABLE(
	${SSBACKUP_EXECUTABLE_NAME}
	WIN32
	MACOSX_BUNDLE
	${SOURCE_FILES}
)

IF(Q_WS_WIN)
	ADD_EXECUTABLE(
		${SSBACKUP_EXECUTABLE_NAME}.com
		${SOURCE_FILES}
	)
	# for second binary
	TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME}.com ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY})
	TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME}.com debug ${QT_QTGUI_LIBRARY_DEBUG} ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTCORE_LIBRARY_DEBUG})
ENDIF(Q_WS_WIN)

TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTCORE_LIBRARY})
TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} debug ${QT_QTGUI_LIBRARY_DEBUG} ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTCORE_LIBRARY_DEBUG})

IF(CMAKE_BUILD_TYPE STREQUAL Deploy)
	IF(Q_WS_WIN)
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} ${QT_GUI_LIB_DEPENDENCIES} ${QT_NETWORK_LIB_DEPENDENCIES} ${QT_CORE_LIB_DEPENDENCIES})
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} ole32 oleaut32 uuid winspool)
		# for second binary
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME}.com ${QT_GUI_LIB_DEPENDENCIES} ${QT_NETWORK_LIB_DEPENDENCIES} ${QT_CORE_LIB_DEPENDENCIES})
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME}.com ole32 oleaut32 uuid winspool)
	ELSEIF(Q_WS_MAC)
		SET(CMAKE_CXX_FLAGS_DEPLOY "${CMAKE_CXX_FLAGS_DEPLOY} -mmacosx-version-min=10.3 -arch ppc -arch i386")
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} ${QT_GUI_LIB_DEPENDENCIES} ${QT_NETWORK_LIB_DEPENDENCIES} ${QT_CORE_LIB_DEPENDENCIES})
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} "-framework AppKit" iconv System)
	ELSE(Q_WS_WIN)
		SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} -static-libgcc -Wl,-Bstatic Xfixes Xrender Xrandr Xcursor Xinerama fontconfig freetype expat stdc++ -Wl,-Bdynamic)
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} ${QT_NETWORK_LIB_DEPENDENCIES} ${QT_CORE_LIB_DEPENDENCIES})
		TARGET_LINK_LIBRARIES(${SSBACKUP_EXECUTABLE_NAME} SM ICE Xi Xext X11 rt m)
	ENDIF(Q_WS_WIN)
ENDIF(CMAKE_BUILD_TYPE STREQUAL Deploy)


SET(APP_DIR bin)
SET(RESOURCE_DIR resource)
IF(Q_WS_MAC)
	SET(APP_DIR ${PROJECT_NAME})
ENDIF(Q_WS_MAC)

INSTALL(TARGETS ${SSBACKUP_EXECUTABLE_NAME} RUNTIME DESTINATION ${APP_DIR})
# for second binary
IF(Q_WS_WIN)
	INSTALL(TARGETS ${SSBACKUP_EXECUTABLE_NAME}.com RUNTIME DESTINATION ${APP_DIR})
ENDIF(Q_WS_WIN)


IF(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/config_reseller")
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/config_reseller" DESTINATION ${APP_DIR})
ENDIF(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/config_reseller")


IF(Q_WS_WIN)
	FILE(GLOB PLINK_BIN ${PROJECT_SOURCE_DIR}/tools/windows-i386/plink-*)
	FILE(GLOB RSYNC_BIN ${PROJECT_SOURCE_DIR}/tools/windows-i386/rsync-*)
	FILE(GLOB SETACL_BIN ${PROJECT_SOURCE_DIR}/tools/windows-i386/setacl-*)
	ADD_CUSTOM_TARGET(copy ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/config/config ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PLINK_BIN} ${CMAKE_CURRENT_BINARY_DIR}/plink.exe
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RSYNC_BIN} ${CMAKE_CURRENT_BINARY_DIR}/rsync.exe
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SETACL_BIN} ${CMAKE_CURRENT_BINARY_DIR}/setacl.exe
	)

	INSTALL(FILES ${PROJECT_SOURCE_DIR}/config/config DESTINATION ${APP_DIR})
	INSTALL(FILES ${translation_binaries} DESTINATION ${APP_DIR})
	
	INSTALL(PROGRAMS 
			${CMAKE_CURRENT_BINARY_DIR}/plink.exe
			${CMAKE_CURRENT_BINARY_DIR}/rsync.exe
			${CMAKE_CURRENT_BINARY_DIR}/setacl.exe
			DESTINATION ${APP_DIR}
		)
ELSEIF(Q_WS_MAC)
	ADD_CUSTOM_TARGET(copy ALL 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/config/config                             ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/plink-*               ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/ssh-*                 ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/tools/mac-universal/rsync-*               ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${translation_binaries}                                         ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/src/icon/mac/${MACOSX_BUNDLE_ICON_FILE}   ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE}
	)
	

	# work around cmake bug
	IF(NOT CMAKE_BUILD_TYPE STREQUAL Deploy)
		SET_TARGET_PROPERTIES(${SSBACKUP_EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-framework QtGui")
	ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL Deploy)

	INSTALL(FILES ${PROJECT_SOURCE_DIR}/config/config DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
	IF(EXISTS ${PROJECT_SOURCE_DIR}/config/config_reseller)
		INSTALL(FILES ${PROJECT_SOURCE_DIR}/config/config_reseller DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
	ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/config/config_reseller)
	INSTALL(FILES ${translation_binaries} DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/plink DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/ssh DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS/rsync DESTINATION ${APP_DIR}/${SSBACKUP_EXECUTABLE_NAME}.app/Contents/MacOS)
ELSE(Q_WS_WIN)
	EXECUTE_PROCESS(COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
	IF(ARCH MATCHES "64$")
		SET(LINUX_DIR linux-x86_64)
	ELSE(ARCH MATCHES "64$")
		SET(LINUX_DIR linux-i386)
	ENDIF(ARCH MATCHES "64$")
	ADD_CUSTOM_TARGET(copy ALL 
		COMMAND cp ${PROJECT_SOURCE_DIR}/config/config ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND cp ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/plink-* ${CMAKE_CURRENT_BINARY_DIR}/plink
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/plink
		COMMAND cp ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/ssh-* ${CMAKE_CURRENT_BINARY_DIR}/ssh
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/ssh
		COMMAND cp ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/rsync-* ${CMAKE_CURRENT_BINARY_DIR}/rsync
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/rsync
		COMMAND cp ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/getfacl-* ${CMAKE_CURRENT_BINARY_DIR}/getfacl
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/getfacl
		COMMAND cp ${PROJECT_SOURCE_DIR}/tools/${LINUX_DIR}/setfacl-* ${CMAKE_CURRENT_BINARY_DIR}/setfacl
		COMMAND chmod u+x ${CMAKE_CURRENT_BINARY_DIR}/setfacl
		
		COMMAND cp ${PROJECT_SOURCE_DIR}/src/icon/linux/${LINUX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.png
	)

	INSTALL(FILES ${PROJECT_SOURCE_DIR}/config/config DESTINATION ${APP_DIR})
	IF(EXISTS ${PROJECT_SOURCE_DIR}/config/config_reseller)
		INSTALL(FILES ${PROJECT_SOURCE_DIR}/config/config_reseller DESTINATION ${APP_DIR})
	ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/config/config_reseller)
	INSTALL(FILES ${translation_binaries} DESTINATION ${APP_DIR})
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/plink   DESTINATION ${APP_DIR})
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ssh     DESTINATION ${APP_DIR})
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rsync   DESTINATION ${APP_DIR})
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/getfacl DESTINATION ${APP_DIR})
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/setfacl DESTINATION ${APP_DIR})
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.png DESTINATION ${RESOURCE_DIR})
ENDIF(Q_WS_WIN)

ADD_CUSTOM_TARGET(
	copyright
)

IF(Q_WS_WIN)
	ADD_CUSTOM_COMMAND(TARGET ${SSBACKUP_EXECUTABLE_NAME}.com POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.com.exe ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.com
		COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/${SSBACKUP_EXECUTABLE_NAME}.com.exe
	)
ENDIF(Q_WS_WIN)
