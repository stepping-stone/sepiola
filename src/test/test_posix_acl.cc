/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#include "test_posix_acl.hh"

#include "tools/posix_acl.hh"

void TestPosixAcl::unescapeOctalCharacters()
{
    bool ok;

    ok = false;
    PosixAcl::unescapeOctalCharacters("/tmp2/uml\\303\\244\\303\\266\\303\\274.txt", &ok);
    QVERIFY(ok);

    ok = false;
    PosixAcl::unescapeOctalCharacters("tmp/permtest/sub1/2.txt", &ok);
    QVERIFY(ok);

    ok = true;
    PosixAcl::unescapeOctalCharacters("/tmp2/uml\\999.txt", &ok);
    QVERIFY(!ok);
}

void TestPosixAcl::getMetadata()
{
    QString desiredDestinationFile("/tmp/getMetadata.test");

    QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems;
    processedItems << qMakePair(QString("/etc/hosts"), AbstractRsync::TRANSFERRED);

    PosixAcl posixAcl("/bin/getfacl", "/bin/setfacl");
    QString destinationFile = posixAcl.getMetadata(desiredDestinationFile, processedItems, 0);

    QCOMPARE(desiredDestinationFile, destinationFile);
}

void TestPosixAcl::setMetadata()
{

}

QTEST_MAIN(TestPosixAcl)
