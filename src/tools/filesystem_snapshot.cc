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

#include <QThread>
#include <QString>
#include <QEventLoop>

#include "filesystem_snapshot.hh"
#include "tools/tool_factory.hh"


FilesystemSnapshot::FilesystemSnapshot( const BackupSelectionHash& includes)
{
    // Save the included files
    this->includeRules = includes;

    // Create a new snapshot thread
    this->snapshotThread = new QThread;

    // Get the snapshot implementation
    this->snapshot = ToolFactory::getSnapshotImpl();

    // Immediately check if the snapshot needs some cleanup
    this->snapshot->checkCleanup();

    // Move the snapshot object to the newly created thread
    this->snapshot->moveToThread( this->snapshotThread );

    // Register the BackupSelectionHash type
    qRegisterMetaType<BackupSelectionHash>("BackupSelectionHash");

    // Connect signals and slots
    QObject::connect( this, SIGNAL( sendCreateSnapshotObject() ),
                      this->snapshot, SLOT( createSnapshotObject() ) );
    QObject::connect( this, SIGNAL( sendInitializeSnapshot() ),
                      this->snapshot, SLOT( initializeSnapshot() ) );
    QObject::connect( this, SIGNAL( sendAddFilesToSnapshot( const BackupSelectionHash ) ),
                      this->snapshot, SLOT( addFilesToSnapshot( const BackupSelectionHash ) ) );
    QObject::connect( this, SIGNAL( sendTakeSnapshot() ),
                      this->snapshot, SLOT( takeSnapshot() ) );
    QObject::connect( this, SIGNAL( sendCleanupSnapshot() ),
                      this->snapshot, SLOT( cleanupSnapshot() ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotObjectCreated(int) ),
                      this, SLOT( snapshotObjectCreated(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotInitialized(int) ),
                      this, SLOT( snapshotInitialized(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendFilesAddedToSnapshot(int) ),
                      this, SLOT( filesAddedToSnapshot(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotTaken(int) ),
                      this, SLOT( snapshotTaken(int) ) );

    // And start the snapshot thread
    this->snapshotThread->start();
}

FilesystemSnapshot::FilesystemSnapshot( )
{
    // Create a new snapshot thread
    this->snapshotThread = new QThread;

    // Get the snapshot implementation
    this->snapshot = ToolFactory::getSnapshotImpl();


    // Immediately check if the snapshot needs some cleanup
    this->snapshot->checkCleanup();

    // Move the snapshot object to the newly created thread
    this->snapshot->moveToThread( this->snapshotThread );

    // Register the BackupSelectionHash type
    qRegisterMetaType<BackupSelectionHash>("BackupSelectionHash");

    // Connect signals and slots
    QObject::connect( this, SIGNAL( sendCreateSnapshotObject() ),
                      this->snapshot, SLOT( createSnapshotObject() ) );
    QObject::connect( this, SIGNAL( sendInitializeSnapshot() ),
                      this->snapshot, SLOT( initializeSnapshot() ) );
    QObject::connect( this, SIGNAL( sendAddFilesToSnapshot( const BackupSelectionHash ) ),
                      this->snapshot, SLOT( addFilesToSnapshot( const BackupSelectionHash ) ) );
    QObject::connect( this, SIGNAL( sendTakeSnapshot() ),
                      this->snapshot, SLOT( takeSnapshot() ) );
    QObject::connect( this, SIGNAL( sendCleanupSnapshot() ),
                      this->snapshot, SLOT( cleanupSnapshot() ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotObjectCreated(int) ),
                      this, SLOT( snapshotObjectCreated(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotInitialized(int) ),
                      this, SLOT( snapshotInitialized(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendFilesAddedToSnapshot(int) ),
                      this, SLOT( filesAddedToSnapshot(int) ) );
    QObject::connect( this->snapshot, SIGNAL( sendSnapshotTaken(int) ),
                      this, SLOT( snapshotTaken(int) ) );

    // And start the snapshot thread
    this->snapshotThread->start();
}

FilesystemSnapshot::~FilesystemSnapshot()
{

    // Disconnect the signals and slots
    QObject::disconnect( this, SIGNAL( sendCreateSnapshotObject() ),
                      this->snapshot, SLOT( createSnapshotObject() ) );
    QObject::disconnect( this, SIGNAL( sendInitializeSnapshot() ),
                      this->snapshot, SLOT( initializeSnapshot() ) );
    QObject::disconnect( this, SIGNAL( sendAddFilesToSnapshot( const BackupSelectionHash ) ),
                      this->snapshot, SLOT( addFilesToSnapshot( const BackupSelectionHash ) ) );
    QObject::disconnect( this, SIGNAL( sendTakeSnapshot() ),
                      this->snapshot, SLOT( takeSnapshot() ) );
    QObject::disconnect( this, SIGNAL( sendCleanupSnapshot() ),
                      this->snapshot, SLOT( cleanupSnapshot() ) );
    QObject::disconnect( this->snapshot, SIGNAL( sendSnapshotObjectCreated(int) ),
                      this, SLOT( this->snapshotObjectCreated(int) ) );
    QObject::disconnect( this->snapshot, SIGNAL( sendSnapshotInitialized(int) ),
                      this, SLOT( this->snapshotInitialized(int) ) );
    QObject::disconnect( this->snapshot, SIGNAL( sendFilesAddedToSnapshot(int) ),
                      this, SLOT( this->filesAddedToSnapshot(int) ) );
    QObject::disconnect( this->snapshot, SIGNAL( sendSnapshotTaken(int) ),
                      this, SLOT( this->snapshotTaken(int) ) );

    // Delete the snapshot thread
    delete this->snapshotThread;

    // Delete the snapshot object
    delete this->snapshot;
}

void FilesystemSnapshot::setIncludeRules( const BackupSelectionHash& includes )
{
    this->includeRules = includes;
}

void FilesystemSnapshot::doSnapshot()
{
    // Simply start the whole snapshot process by sending the
    // createSnapshotObject signal to the snapshot thread
    emit infoSignal(tr("Taking filesystem snapshot, this might take some time ..."));
    emit sendCreateSnapshotObject();
}

void FilesystemSnapshot::cleanup()
{
    // Emit the cleanup signal for the specific snapshot implementation and
    // wait for the signal that the snapshot has been cleaned up
    QEventLoop loop;
    loop.connect( this->snapshot, SIGNAL( sendSnapshotCleandUp( int ) ), SLOT( quit() ) );
    emit sendCleanupSnapshot();

    // Wait for the cleanup process to finish
    loop.exec();
}

void FilesystemSnapshot::snapshotObjectCreated(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot object successfully created,"
                  << "going to initialize filesystem snapshot";
        emit sendInitializeSnapshot();
    } else
    {
        qDebug() << "Cannot create filesystem snapshot object";
        emit infoSignal(tr("Cannot create filesystem snapshot, stopping backup here"));
        emit sendSnapshotDone( result );
    }
}

void FilesystemSnapshot::snapshotInitialized(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot successfully initialized,"
                  << "going to add backup selection to the filesystem snapshot"
                  << "set";
        emit sendAddFilesToSnapshot( this->includeRules );
    } else
    {
        qDebug() << "Cannot initialize filesytem snapshot";
        emit infoSignal(tr("Cannot create filesystem snapshot (initialization failed), stopping backup here"));
        emit sendSnapshotDone( result );
    }
}

void FilesystemSnapshot::filesAddedToSnapshot(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Successfully added the backup selection to the filesystem"
                  << "snapshot set, going to take the snapshot";
        emit sendTakeSnapshot();
    } else
    {
        qDebug() << "Adding files to snapshot set failed";
        emit infoSignal(tr("Cannot create filesystem snapshot (adding partition to snapshot failed), stopping backup here"));
        emit sendSnapshotDone( result );
    }
}

void FilesystemSnapshot::snapshotTaken(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot successfully taken, going to upload"
                  << "the files to the backup server";
        emit infoSignal(tr("Filesystem snapshot successfully taken"));
        emit sendSnapshotDone( result );

    } else
    {
        qDebug() << "Filesystem snapshot not successfully created";
        emit infoSignal(tr("Cannot create filesystem snapshot, stopping backup here"));
        emit sendSnapshotDone( result );
    }
}

const SnapshotMapper& FilesystemSnapshot::getSnapshotPathMappers() const
{
    return this->snapshot->getSnapshotPathMappers();
}
