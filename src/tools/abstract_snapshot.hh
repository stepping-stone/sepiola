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

#ifndef ABSTRACT_SNAPSHOT_HH_
#define ABSTRACT_SNAPSHOT_HH_

#include <QList>
#include <QHash>
#include <QString>

#include "tools/abstract_informing_process.hh"
#include "tools/filesystem_snapshot_path_mapper.hh"

#define SNAPSHOT_SUCCESS 0
#define SNAPSHOT_CANNOT_CREATE_SNASPHOT_OBJECT 1
#define SNAPSHOT_CANNOT_INITIALIZE_BACKUP 2
#define SNAPSHOT_CANNOT_CREATE_SNAPSHOT 3
#define SNAPSHOR_CANNOT_MOUNT_SNAPSHOT 4

typedef QHash<QString,FilesystemSnapshotPathMapper> SnapshotMapper;

/**
 * The AbstractSnapshot class provides methods for using a snapshot object
 * @author Pat Kläy, pat.klaey@stepping-stone.ch
 */
class AbstractSnapshot : public AbstractInformingProcess
{
    Q_OBJECT

public:

    /**
     * Destroys the AbstractScheduler
     */
    virtual ~AbstractSnapshot();


    virtual const SnapshotMapper& getSnapshotPathMappers() const = 0;

private:
    SnapshotMapper snapshotPathMappers;

signals:

    /**
     * Send the signal that the snapshot object has been created
     * @param Result of the snapshot object creation
     */
    void sendSnapshotObjectCreated(int result);

    /**
     * Send the signal that the snapshot has been initialized
     * @param Result of the snapshot initialization
     */
    void sendSnapshotInitialized(int result);

    /**
     * Send the signal that the files have been added to the snapshot selection
     * @param Result of the files adding process
     */
    void sendFilesAddedToSnapshot(int result);

    /**
     * Send the signal that the snapshot has been taken
     * @param Result of the snapshot process
     */
    void sendSnapshotTaken(int result);

    /**
     * Send the signal that the snapshot has been cleaned up
     * @param Result of the cleanup
     */
    void sendSnapshotCleandUp(int result);

    /**
     * Send the signal whether or not the snapshot needs some cleanup
     * @param Whether the snapshot needs some cleanup or not
     */
    void sendSnapshotNeedsCleanup(bool cleanup);

public slots:

    /**
     * Creates a new snapshot object
     */
    virtual void createSnapshotObject() = 0;

    /**
     * Initializes the snapshot object
     */
    virtual void initializeSnapshot() = 0;

    /**
     * Adds all the given files to the snapshot selection
     * @param The BackupSelectionHash which defines all files which are later
     * backed-up
     */
    virtual void addFilesToSnapshot( const BackupSelectionHash includeRules ) = 0;

    /**
     * Executes the snapshot
     */
    virtual void takeSnapshot() = 0;

    /**
     * Cleans up the snapshot
     */
    virtual void cleanupSnapshot() = 0;
};

inline AbstractSnapshot::~AbstractSnapshot()
{
}

#endif /* ABSTRACT_SNAPSHOT_HH_ */
