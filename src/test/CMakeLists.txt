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

# Unix specific tests

IF(Q_WS_X11)

# TODO:
# - remove Settings::SHOW_PROGRESS in process.cc such that we can
#   remove settings.{cc,hh} and it's rev-deps here

QT4_WRAP_CPP(test_posix_acl_MOC
    test_posix_acl.hh
    ../tools/posix_acl.hh
    ../tools/abstract_informing_process.hh
    ../utils/log_file_utils.hh
    ../settings/settings.hh
    )

ADD_EXECUTABLE(test_posix_acl test_posix_acl.cc
    ${test_posix_acl_MOC}
    ../tools/posix_acl.cc
    ../tools/process.cc
    ../utils/log_file_utils.cc
    ../utils/file_system_utils.cc
    ../utils/string_utils.cc
    ../settings/settings.cc
    ../model/backup_task.cc
    ../model/scheduled_task.cc
    )

TARGET_LINK_LIBRARIES(test_posix_acl
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY})

ADD_TEST(test_posix_acl ${CMAKE_CURRENT_BINARY_DIR}/test_posix_acl)

ENDIF(Q_WS_X11)

# All-platform tests

QT4_WRAP_CPP(test_host_file_utils_MOC
    test_host_file_utils.hh
    ../utils/host_file_utils.hh
    )

ADD_EXECUTABLE(test_host_file_utils test_host_file_utils.cc
    ${test_host_file_utils_MOC}
    ../utils/host_file_utils.cc
    ../utils/file_system_utils.cc
    )

TARGET_LINK_LIBRARIES(test_host_file_utils
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY})

ADD_TEST(test_host_file_utils ${CMAKE_CURRENT_BINARY_DIR}/test_host_file_utils)
