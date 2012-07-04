/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2012 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#include "test_host_file_utils.hh"

#include "utils/host_file_utils.hh"

void TestHostFileUtils::getPuttyKey()
{

    /*
    QString key = HostFileUtils::getPuttyKey("testhost");
    QVERIFY(!key.isEmpty());
    */
    QFAIL("not implemented");
}

void TestHostFileUtils::convertPuttyKey()
{
    QFAIL("not implemented");
}

void TestHostFileUtils::addPuttyKeyToOpenSSHKeyFile()
{
    QFAIL("not implemented");
}

QTEST_MAIN(TestHostFileUtils)
