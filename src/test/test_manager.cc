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

#include <QDebug>
#include <QString>
#include <QStringList>

#include "test_manager.hh"

namespace {
const QString TEST_ARGUMENT = "-test";
}

bool TestManager::isTestApplication(int argc, char *argv[])
{
    return getTests(argc, argv).size() > 0;
}

void TestManager::run(int argc, char *argv[])
{
    QStringList testList = getTests(argc, argv);
    qDebug() << "Run " << testList.size() << " test(s)";
    foreach (QString testName, testList) {
        qDebug() << "Run test " << testName;
        runTest(testName);
        qDebug() << "Finished test " << testName;
    }
    qDebug() << "All tests done.";
}

QStringList TestManager::getTests(int argc, char *argv[])
{
    QStringList testList;
    for (int i = 1; i < argc; i++) {
        if (TEST_ARGUMENT.compare(argv[i]) == 0) {
            testList << argv[++i];
        }
    }
    return testList;
}
