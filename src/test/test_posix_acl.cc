/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2012 stepping stone GmbH
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
