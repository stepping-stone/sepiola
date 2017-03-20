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

#ifndef FILESYSTEM_SNAPSHOT_HH_
#define FILESYSTEM_SNAPSHOT_HH_

#include "tools/abstract_snapshot.hh"

// Forward declarations
class QThread;
class QString;

/**
 * The filesystem snapshot class is a wrapper which serves the client with
 * operating system specific snapshot implementation
 * @author Pat Kläy, pat.klaey@stepping-stone.ch
 */
class FilesystemSnapshot : public QObject
{

    Q_OBJECT

public:
    FilesystemSnapshot( const BackupSelectionHash& includes );
    FilesystemSnapshot( );
    virtual ~FilesystemSnapshot();
    void doSnapshot();
    const SnapshotMapper& getSnapshotPathMappers() const;
    void setIncludeRules( const BackupSelectionHash& includes );
    void cleanup();

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
    void sendCleanupSnapshot();

    // Common signals
    void infoSignal( const QString& text );
    void errorSignal( const QString& text );

private:
    AbstractSnapshot * snapshot;
    QThread * snapshotThread;
    BackupSelectionHash includeRules;
};

#endif /* FILESYSTEM_SNAPSHOT_HH_ */
