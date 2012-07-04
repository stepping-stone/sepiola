QT4_WRAP_CPP(test_posix_acl_MOC
    test_posix_acl.hh
    ../tools/posix_acl.hh
    ../tools/abstract_informing_process.hh
    ../utils/log_file_utils.hh
    )

ADD_EXECUTABLE(test_posix_acl test_posix_acl.cc
    ${test_posix_acl_MOC}
    ../tools/posix_acl.cc
    ../tools/process.cc
    ../utils/log_file_utils.cc
    ../utils/file_system_utils.cc
    ../utils/string_utils.cc
    )

TARGET_LINK_LIBRARIES(test_posix_acl
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY})

ADD_TEST(test_posix_acl ${CMAKE_CURRENT_BINARY_DIR}/test_posix_acl)
