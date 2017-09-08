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

#include "optionalSnapshot.hh"
#include "settings/settings.hh"
#include "utils/file_system_utils.hh"

#include <QString>

OptionalSnapshot::OptionalSnapshot(shared_ptr<AbstractSnapshot> ptr) :
    _snapshot(ptr)
{
    QObject::connect( _snapshot.get(), SIGNAL( sendSnapshotObjectCreated(int) ),
                      this, SIGNAL( sendSnapshotObjectCreated(int) ) );

    QObject::connect( _snapshot.get(), SIGNAL( sendSnapshotInitialized(int) ),
                      this, SIGNAL( sendSnapshotInitialized(int) ) );

    QObject::connect( _snapshot.get(), SIGNAL( sendFilesAddedToSnapshot(int) ),
                      this, SIGNAL( sendFilesAddedToSnapshot(int) ) );

    QObject::connect( _snapshot.get(), SIGNAL( sendSnapshotTaken(int) ),
                      this, SIGNAL( sendSnapshotTaken(int) ) );

    QObject::connect( _snapshot.get(), SIGNAL( sendSnapshotCleandUp(int) ),
                      this, SIGNAL( sendSnapshotCleandUp(int) ) );
}

OptionalSnapshot::~OptionalSnapshot()
{
    QObject::disconnect( _snapshot.get(), SIGNAL( sendSnapshotObjectCreated(int) ),
                      this, SIGNAL( sendSnapshotObjectCreated(int) ) );

    QObject::disconnect( _snapshot.get(), SIGNAL( sendSnapshotInitialized(int) ),
                      this, SIGNAL( sendSnapshotInitialized(int) ) );

    QObject::disconnect( _snapshot.get(), SIGNAL( sendFilesAddedToSnapshot(int) ),
                      this, SIGNAL( sendFilesAddedToSnapshot(int) ) );

    QObject::disconnect( _snapshot.get(), SIGNAL( sendSnapshotTaken(int) ),
                      this, SIGNAL( sendSnapshotTaken(int) ) );

    QObject::disconnect( _snapshot.get(), SIGNAL( sendSnapshotCleandUp(int) ),
                      this, SIGNAL( sendSnapshotCleandUp(int) ) );
}

void OptionalSnapshot::createSnapshotObject()
{
    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->createSnapshotObject();
    } else {
        // Nothing to do here, simply send the signal that the object has been
        // created
        emit sendSnapshotObjectCreated( SNAPSHOT_SUCCESS );
    }
}

void OptionalSnapshot::initializeSnapshot()
{
    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->initializeSnapshot();
    } else {
        // Simply send the snapshot initialized signal
        emit sendSnapshotInitialized( SNAPSHOT_SUCCESS );
    }
}

void OptionalSnapshot::addFilesToSnapshot(const BackupSelectionHash includeRules)
{
    this->_snapshotPathMappers.clear();

    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->addFilesToSnapshot(includeRules);
    } else {
        foreach( QString file, includeRules.keys() )
        {
            // Get the driveletter from the root item e.g:  C:\ -> C
            QString driveLetter = FileSystemUtils::getDriveLetterByFile(file);

            // Get the the file path without the patrition.
            QString relativeFileName = file;
            relativeFileName.replace( driveLetter + ":/", QString(""));

            // Check if the corresponding entry in the SnapshotMapper already exists
            if ( this->_snapshotPathMappers.contains( driveLetter ) )
            {
                FilesystemSnapshotPathMapper mapper = this->_snapshotPathMappers.value( driveLetter );
                if (!relativeFileName.isEmpty())
                    mapper.addFileToRelativeIncludes( relativeFileName, includeRules[file]);
                mapper.setSnapshotPath(driveLetter + ":\\");
                this->_snapshotPathMappers.insert (driveLetter, mapper );
            } else
            {
                QHash<QString,bool> empty;
                FilesystemSnapshotPathMapper mapper(driveLetter, empty);
                if (!relativeFileName.isEmpty())
                    mapper.addFileToRelativeIncludes( relativeFileName, includeRules[file]);
                mapper.setSnapshotPath(driveLetter + ":\\");
                this->_snapshotPathMappers.insert( driveLetter, mapper );
            }
        }
        emit sendFilesAddedToSnapshot( SNAPSHOT_SUCCESS );
    }
}

void OptionalSnapshot::takeSnapshot()
{
    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->takeSnapshot();
    } else {
        // Again, no action needed, just send the signal that the snapshot is taken
        emit sendSnapshotTaken( SNAPSHOT_SUCCESS );
    }
}

void OptionalSnapshot::cleanupSnapshot()
{
    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->cleanupSnapshot();
    } else {
        // Nothing to do as nothing was done while taking the snapshot, just send
        // the signal to continue
        emit sendSnapshotCleandUp( SNAPSHOT_SUCCESS );
    }
}

const SnapshotMapper& OptionalSnapshot::getSnapshotPathMappers() const
{
    if (Settings::getInstance()->getDoSnapshot()) {
        return _snapshot->getSnapshotPathMappers();
    } else
        return this->_snapshotPathMappers;
}

void OptionalSnapshot::checkCleanup() {

    if (Settings::getInstance()->getDoSnapshot()) {
        _snapshot->checkCleanup();
    } else {
        // Nothing to do as nothing was done while taking the snapshot, just send
        // the signal to continue
        emit sendSnapshotCleandUp( SNAPSHOT_SUCCESS );
    }
}
