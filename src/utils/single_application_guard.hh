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
#|
#|
#|
#| This class takes the responsibility for running sepiola as a single instance.
#| This is based on ideas and code from http://aeguana.com/blog/how-to-run-a-single-app-instance-in-qt/
*/

#ifndef SINGLE_APPLICATION_GUARD_HH
#define SINGLE_APPLICATION_GUARD_HH

#include <QSharedMemory>


class SingleApplicationGuard
{

public:
    SingleApplicationGuard( const QString& key );
    ~SingleApplicationGuard();

    bool tryToCreateSharedMemory();


private:
    void _releaseMemory();

    const QString _userName;
    const QString _sharedmemKey;
    QSharedMemory _sharedMem;

};
#endif // SINGLE_APPLICATION_GUARD_HH
