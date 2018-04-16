/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2017 stepping stone GmbH
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

#ifndef TEST_HOST_FILE_UTILS_HH
#define TEST_HOST_FILE_UTILS_HH

#include <QtTest/QTest>

class TestHostFileUtils : public QObject
{
    Q_OBJECT
private slots:
    void getPuttyKey();
    void convertPuttyKey();
    void addPuttyKeyToOpenSSHKeyFile();
};

#endif // TEST_HOST_FILE_UTILS_HH
