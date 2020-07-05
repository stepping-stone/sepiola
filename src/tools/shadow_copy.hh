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

#ifndef SHADOW_COPY_HH_
#define SHADOW_COPY_HH_

#include "abstract_snapshot.hh"

#include "error.h"
#include <iostream>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <QHash>
#include <QString>

/* Define some vss snapshot errors code which are 100 < code < 200*/
#define SNAPSHOT_CANNOT_SET_BACKUP_CONTEXT 101
#define SNAPSHOT_WRITER_GATHERING_METADATA_FAILED 102
#define SNAPSHOT_ASYNC_WAIT_FAILED 103
#define SNAPSHOT_CANNOT_START_SNAPSHOT_SET 104
#define SNAPSHOT_CANNOT_ADD_PARTITION_TO_SNAPSHOT_SET 105
#define SNAPSHOT_CANNOT_SET_SNAPSHOT_STATE 106
#define SNAPSHOT_CANNOT_PREPARE_FOR_BACKUP 107
#define SNAPSHOT_CANNOT_GET_SNAPSHOT_PROPERTIES 108

/**
 * The dummy snapshot class provides an empty snapshot method
 * @author Pat Kläy, pat.klaey@stepping-stone.ch
 */
class ShadowCopy : public AbstractSnapshot
{
    Q_OBJECT

public:
    /**
     * Creates the ShadowCopy object
     */
    ShadowCopy();

    /**
     * Destroys the ShadowCopy object
     */
    virtual ~ShadowCopy();

    const SnapshotMapper &getSnapshotPathMappers() const;

    /**
     * Checks if there is something to clean up
     */
    virtual void checkCleanup();

public slots:

    /**
     * Creates a new snapshot object
     */
    void createSnapshotObject();

    /**
     * Initializes the snapshot object
     */
    void initializeSnapshot();

    /**
     * Adds all the given files to the snapshot selection
     * @param The BackupSelectionHash which defines all files which are later
     * backed-up
     */
    void addFilesToSnapshot(const BackupSelectionHash includeRules);

    /**
     * Executes the snapshot
     */
    void takeSnapshot();

    /**
     * Cleans up the snapshot
     */
    virtual void cleanupSnapshot();

private:
    QString wCharArrayToQString(WCHAR *string);
    bool removeWindowsSymlink(QString linkname);
    QString getMountDirectory();
    SnapshotMapper snapshotPathMappers;
    static const _VSS_SNAPSHOT_CONTEXT SC_SNAPSHOT_CONTEXT = VSS_CTX_BACKUP;
    static const VSS_BACKUP_TYPE SC_SNAPSHOT_TYPE = VSS_BT_COPY;
    static const bool SC_SNAPSHOT_BOOTABLE_STATE = false;
    static const bool SC_SNAPSHOT_SELECT_COMPONENTS = false;
    static const QString MOUNT_PREFIX;

    HMODULE vssapiBase;
    IVssBackupComponents *pBackup = NULL;
    IVssAsync *pAsync = NULL;
    IVssAsync *pPrepare = NULL;
    IVssAsync *pDoShadowCopy = NULL;
    QHash<QString, VSS_ID> snapshot_set_ids;
};

#endif /* SHADOW_COPY_HH_ */
