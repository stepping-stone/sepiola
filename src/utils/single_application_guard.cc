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
*/

#include <QCryptographicHash>
#include <QDebug>

#include "single_application_guard.hh"

namespace {

QString generateUserDependingHash(const QString &user)
{
    QByteArray data;

    data.append(user);
    data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
    return data;
}

} // namespace

SingleApplicationGuard::SingleApplicationGuard(const QString &userName)
    : _userName(userName)
    , _sharedmemKey(generateUserDependingHash(_userName))
    , _sharedMem(generateUserDependingHash(_userName))
{
#ifndef Q_OS_WIN32
    // On unix systems shared memory is not freed upon crash.
    // So if there is any trash form previous instance clean it.
    QSharedMemory memoryToClear(_sharedmemKey);
    memoryToClear.lock();
    if (memoryToClear.attach()) {
        memoryToClear.detach();
    }
    memoryToClear.unlock();
#endif
}

SingleApplicationGuard::~SingleApplicationGuard()
{
    _releaseMemory();
}

bool SingleApplicationGuard::tryToCreateSharedMemory()
{
    /**
     *Try to create and attach a shared memory segment with the key passed to the constructor.
     *This is based on ideas and code from
     *http://aeguana.com/blog/how-to-run-a-single-app-instance-in-qt/
     */
    _sharedMem.lock();
    const bool isSharedMemFree = _sharedMem.create(sizeof(quint64));
    _sharedMem.unlock();
    if (!isSharedMemFree) {
        _releaseMemory();
        return false;
    }
    return true;
}

void SingleApplicationGuard::_releaseMemory()
{
    if (_sharedMem.isAttached())
        _sharedMem.detach();
    _sharedMem.unlock();
}
