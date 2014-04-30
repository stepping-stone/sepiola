/*
 * filesystem_snapshot.hh
 *
 *  Created on: Apr 29, 2014
 *      Author: pat
 */

#ifndef FILESYSTEM_SNAPSHOT_HH_
#define FILESYSTEM_SNAPSHOT_HH_

#include "tools/abstract_snapshot.hh"

// Forward declarations
class QThread;
class QString;

class FilesystemSnapshot : public QObject
{

    Q_OBJECT

public:
    FilesystemSnapshot( const BackupSelectionHash& includes );
    virtual ~FilesystemSnapshot();
    void doSnapshot();
    const QList<FilesystemSnapshotPathMapper>& getSnapshotPathMappers();

private slots:
    void snapshotObjectCreated( int result );
    void snapshotInitialized( int result );
    void filesAddedToSnapshot( int result );
    void snapshotTaken( int result );

signals:
    void sendCreateSnapshotObject();
    void sendInitializeSnapshot();
    void sendAddFilesToSnapshot( const BackupSelectionHash includeRules );
    void sendTakeSnapshot();
    void sendSnapshotDone( int result );

    // Common signals
    void infoSignal( const QString& text );
    void errorSignal( const QString& text );

private:
    AbstractSnapshot * snapshot;
    QThread * snapshotThread;
    BackupSelectionHash includeRules;
};

#endif /* FILESYSTEM_SNAPSHOT_HH_ */
