/*
 * filesystem_snapshot.cc
 *
 *  Created on: Apr 29, 2014
 *      Author: pat
 */

#include <QThread>
#include <QString>

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

void FilesystemSnapshot::doSnapshot()
{
    // Simply start the whole snapshot process by sending the
    // createSnapshotObject signal to the snapshot thread
    emit sendCreateSnapshotObject();
}

QString FilesystemSnapshot::getSnapshotPathForFile( QString file )
{
    // Call the snapshots getFileSnapshotPath method to obtain the correct
    // location on the filesystem
    return this->snapshot->getFileSnapshotPath( file );
}

void FilesystemSnapshot::snapshotObjectCreated(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot object successfully created, "
                  << "going to initialize filesystem snapshot";
        emit sendInitializeSnapshot();
    } else
    {
        // TODO, delete the snapshot object and proceed or abort the backup process
    }
}

void FilesystemSnapshot::snapshotInitialized(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot successfully initialized, "
                  << "going to add backup selection to the filesystem snapshot "
                  << "set";
        emit sendAddFilesToSnapshot( this->includeRules );
    } else
    {
        // TODO, delete the snapshot object and proceed or abort the backup process
    }
}

void FilesystemSnapshot::filesAddedToSnapshot(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Successfully added the backup selection to the filesystem"
                  << " snapshot set, going to take the snapshot";
        emit sendTakeSnapshot();
    } else
    {
        // TODO, delete the snapshot object and proceed or abort the backup process
    }
}

void FilesystemSnapshot::snapshotTaken(int result)
{
    if ( result == SNAPSHOT_SUCCESS )
    {
        qDebug() << "Filesystem snapshot successfully taken, going to upload "
                  << "the files to the backup server";

        emit sendSnapshotDone( result );

    } else
    {
        // TODO, delete the snapshot object and proceed or abort the backup process
    }
}


