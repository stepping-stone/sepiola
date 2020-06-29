/*
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
*/

#include "test_host_file_utils.hh"

#include "utils/host_file_utils.hh"

#include <QTextStream>
#include <QTemporaryFile>

void TestHostFileUtils::getPuttyKey()
{

    /*
    QString key = HostFileUtils::getPuttyKey("testhost");
    QVERIFY(!key.isEmpty());
    */
    QSKIP("not implemented", SkipSingle);
}

void TestHostFileUtils::convertPuttyKey()
{
    QSKIP("not implemented", SkipSingle);
}

void TestHostFileUtils::addPuttyKeyToOpenSSHKeyFile()
{
#ifdef Q_OS_WIN32
    QSKIP("not implemented (can't fake-read from registry)", SkipSingle);
#else
	QTemporaryFile
		putty("putty-sshhostkeys"),
		testhost1("ssh-known_hosts-testhost1"),
		testhost2("ssh-known_hosts-testhost2"),
		invalidhost("ssh-known_hosts-invalid");

	if (putty.open()) {
		QTextStream stream(&putty);
		// HostKey with a legacy/small exponent:
		stream << "rsa2@22:testhost1.invalid.domain 0x23,0xc0b87f346acc9af567c8b2af13af01c56303f0a2bd1039a87e901851376eb531b014ce2ec81420669c00feaa772b41307fdbbbabb6131a69c34682677dedb7d7629df1388861588632d7d2ec0c22b17d65ae3ced72c393209ecadfca1c692254af8a37d806996412f303fa602fac0c39a33d959f6b977e9c95af1f1ccb9a52b4be63b28af6c08c965965c7a16561555e2d71c48f2d70c7a06b2ab77246453c384996869527a2081e7f1c8fcfc38dc1432441318a325f749f7f7570e2c692d8876c69ec964685831a3d66f84c9c0c9abea0199f978cd2a2b31ba8bc1c2ab2f65ee0c64c6d64261bb5a4fc79933888d32dbdc3d8ee69942fb8ead42df8de1c6abf" << endl;
		// HostKeys with a larger exponent:
		stream << "rsa2@22:testhost2.invalid.domain 0x10001,0xcd7843370db604602b9b3766d3871f022c2f5fb3279eb98c18dc22037dc39e71c959bf1588635cfeca2bdb7e05d6ce4a421318665b90cab5003478669884e54cd37968e5cd88677238385991938bf6cf69e70bbf0c395bc7d54ba512a4242057e2b551ff7bd47a128a0287d700aac06216033628f78e6bc065580f634f21831e853f29e025b28a0fd32664bc8624a2ae548cddfb561498bd1f80c22f2d5e335fa189d8e9b6fc4614981e69846ea5a9b3cca6ee230ec79a2e2bb6037b1c84a0dc8004950be910f594a9ce48b7c3a6e81479dcace8e3a68b467f88cbc791f0505ec9e1a2c2e88359c5ae3be08a08ea2f985feddeaec40b174d48445cc7b0018c51" << endl;
		putty.close();
	}
	else
	    QFAIL("writing Putty sshhostkeys failed");

	if (testhost1.open())
		testhost1.close();
	else
	    QFAIL("creating test file for testhost1 failed");

	if (testhost2.open())
		testhost2.close();
	else
	    QFAIL("creating test file for testhost2 failed");

	if (invalidhost.open())
		invalidhost.close();
	else
	    QFAIL("creating test file for invalidhost failed");

	HostFileUtils::addPuttyKeyToOpenSshKeyFile("testhost1.invalid.domain", putty.fileName(), testhost1.fileName());
	if (testhost1.open())
	{
		QCOMPARE(
			QString(testhost1.readLine()),
			QString("testhost1.invalid.domain, ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEAwLh/NGrMmvVnyLKvE68BxWMD8KK9EDmofpAYUTdutTGwFM4uyBQgZpwA/qp3K0Ewf9u7q7YTGmnDRoJnfe2312Kd8TiIYViGMtfS7AwisX1lrjztcsOTIJ7K38ocaSJUr4o32AaZZBLzA/pgL6wMOaM9lZ9rl36cla8fHMuaUrS+Y7KK9sCMllllx6FlYVVeLXHEjy1wx6BrKrdyRkU8OEmWhpUnoggefxyPz8ONwUMkQTGKMl90n391cOLGktiHbGnslkaFgxo9ZvhMnAyavqAZn5eM0qKzG6i8HCqy9l7gxkxtZCYbtaT8eZM4iNMtvcPY7mmUL7jq1C343hxqvw==\n"));
		testhost1.close();
	}
	else
	    QFAIL("opening test file for testhost1 failed");

	HostFileUtils::addPuttyKeyToOpenSshKeyFile("testhost2.invalid.domain", putty.fileName(), testhost2.fileName());
	if (testhost2.open())
	{
		QCOMPARE(
			QString(testhost2.readLine()),
			QString("testhost2.invalid.domain, ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDNeEM3DbYEYCubN2bThx8CLC9fsyeeuYwY3CIDfcOecclZvxWIY1z+yivbfgXWzkpCExhmW5DKtQA0eGaYhOVM03lo5c2IZ3I4OFmRk4v2z2nnC78MOVvH1UulEqQkIFfitVH/e9R6EooCh9cAqsBiFgM2KPeOa8BlWA9jTyGDHoU/KeAlsooP0yZkvIYkoq5UjN37VhSYvR+Awi8tXjNfoYnY6bb8RhSYHmmEbqWps8ym7iMOx5ouK7YDexyEoNyABJUL6RD1lKnOSLfDpugUedys6OOmi0Z/iMvHkfBQXsnhosLog1nFrjvgigjqL5hf7d6uxAsXTUhEXMewAYxR\n"));
		testhost2.close();
	}
	else
	    QFAIL("opening test file for testhost2 failed");

	HostFileUtils::addPuttyKeyToOpenSshKeyFile("invalidhost.invalid.domain", putty.fileName(), invalidhost.fileName());
	if (invalidhost.open())
	{
		QString line = invalidhost.readLine();
		QVERIFY(line.isEmpty()); // there should be no line written for an invalid host
		invalidhost.close();
	}
	else
	    QFAIL("opening test file for invalidhost failed");
#endif
}

QTEST_MAIN(TestHostFileUtils)
