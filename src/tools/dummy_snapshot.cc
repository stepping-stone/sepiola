/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2014 stepping stone GmbH
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

#include "dummy_snapshot.hh"

DummySnapshot::DummySnapshot()
{
    // Empty constructor
}

DummySnapshot::~DummySnapshot()
{
    // As the constructor is empty, the destructor can be empty too
}

void DummySnapshot::createSnapshotObject()
{
    // Nothing to do here, simply send the signal that the object has been
    // created
    emit sendSnapshotObjectCreated( SNAPSHOT_SUCCESS );
}

void DummySnapshot::initializeSnapshot()
{
    // Simply send the snapshot initialized signal (do nothing at all)
    emit sendSnapshotInitialized( SNAPSHOT_SUCCESS );
}

void DummySnapshot::addFilesToSnapshot(const BackupSelectionHash includeRules)
{
    // Again nothing to do here, just send the signal that the files were added
    emit sendFilesAddedToSnapshot( SNAPSHOT_SUCCESS );
}

void DummySnapshot::takeSnapshot()
{
    // Again, no action needed, just send the signal that the snapshot is taken
    emit sendSnapshotTaken( SNAPSHOT_SUCCESS );
}
