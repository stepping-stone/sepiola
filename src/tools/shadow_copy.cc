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

#include "shadow_copy.hh"
#include "settings/settings.hh"
#include "utils/file_system_utils.hh"

#include <QString>
#include <QDebug>
#include <QRegExp>
#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QWaitCondition>
#include <QMutex>

#ifndef __GNUC__
// only valid for Visual C++ linker (either with Visual C++ or clang frontend)
#pragma comment (lib, "VssApi.lib")
#endif

/* Functions in VSSAPI.DLL */
typedef HRESULT(STDAPICALLTYPE * _CreateVssBackupComponentsInternal)(
    OUT IVssBackupComponents** ppBackup);
typedef void(APIENTRY * _VssFreeSnapshotPropertiesInternal)(IN VSS_SNAPSHOT_PROP* pProp);
static _CreateVssBackupComponentsInternal CreateVssBackupComponentsInternal_I;

/* Funktions in kernel32.dll */
typedef BOOL (WINAPI* CreateSymbolicLinkProc) (LPCSTR, LPCSTR, DWORD);
typedef BOOL (WINAPI* RemoveDirectoryProc) (LPCSTR);
typedef BOOL (WINAPI* PathFileExistsProc) (LPCSTR);

ShadowCopy::ShadowCopy()
{
    // Constructor
}

ShadowCopy::~ShadowCopy()
{
    // Release the shadow copy creation request
    if (this->pDoShadowCopy != NULL)
        this->pDoShadowCopy->Release();

    // Release the prepeare for backup request
    if (this->pPrepare != NULL)
        this->pPrepare->Release();

    // Release the metadata gathering request
    if (this->pAsync != NULL)
        pAsync->Release();

    // Release the main VSS request
    if (this->pBackup != NULL)
        this->pBackup->Release();
}

void ShadowCopy::createSnapshotObject()
{
    CoInitialize(NULL);

    // Load the vssapi library
    this->vssapiBase = LoadLibrary("vssapi.dll");

    // Get the
    CreateVssBackupComponentsInternal_I = (_CreateVssBackupComponentsInternal)GetProcAddress(this->vssapiBase, "CreateVssBackupComponentsInternal");

    // Create the shadow copy backup component (VSSBackupComponent)
    HRESULT result = CreateVssBackupComponentsInternal_I(&(this->pBackup));

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Cannot create VSSBackupComponent, operation failed with error: 0x%08lx").arg(result);
        qCritical() << error;
        emit sendSnapshotObjectCreated( SNAPSHOT_CANNOT_CREATE_SNASPHOT_OBJECT );
        return;
    }
    emit sendSnapshotObjectCreated( SNAPSHOT_SUCCESS );
}

void ShadowCopy::initializeSnapshot()
{
    // Initialize the backup
    if ( this->pBackup == NULL )
    {
        QString error = QString("Cannot initialize backup, VSSBackupComponent is NULL");
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_INITIALIZE_BACKUP );
        return;
    }

    HRESULT result = this->pBackup->InitializeForBackup();

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Initialize for backup failed with error: = 0x%08lx").arg(result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_INITIALIZE_BACKUP );
        return;
    }

    // Set the context, we want an non-persistant backup which involves writers
    result = this->pBackup->SetContext(this->SC_SNAPSHOT_CONTEXT);

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Setting backup context to %i failed with error: 0x%08lx").arg(this->SC_SNAPSHOT_CONTEXT).arg(result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_SET_BACKUP_CONTEXT );
        return;
    }

    // Tell the writers to gather metadata
    result = this->pBackup->GatherWriterMetadata(&(this->pAsync));

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Writers gathering metadata failed with error: 0x%08lx").arg(result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_WRITER_GATHERING_METADATA_FAILED );
        return;
    }

    // Wait until all writers collected the metadata
    this->pAsync->Wait();

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Waiting for writers collecting metadata failed with error: 0x%08lx").arg(result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    VSS_ID tmp_snapshot_set_id;

    // Start the snapshot set
    result = this->pBackup->StartSnapshotSet(&tmp_snapshot_set_id);

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Starting snapshot set failed with error: 0x%08lx").arg(result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_START_SNAPSHOT_SET );
        return;
    }

    emit sendSnapshotInitialized( SNAPSHOT_SUCCESS );
}

void ShadowCopy::addFilesToSnapshot( const BackupSelectionHash includeRules )
{
    // Go through all files and add the corresponding partition name to the hash
    foreach( QString file, includeRules.keys() )
    {
        qDebug() << "Adding file" << file << "to the snapshot mapper";

        // Get the driveletter of the current file
        QString driveLetter = FileSystemUtils::getDriveLetterByFile( file );

        // Get the relative filename
        QString relativeFileName = file;
        relativeFileName.replace( driveLetter + ":/", QString(""));

        qDebug() << "Relative file name now is:" << relativeFileName;

        // Check if the corresponding entry in the SnapshotMapper already
        // exists
        if ( this->snapshotPathMappers.contains( driveLetter ) )
        {
            // If it exists simply add the file to the list of files of the
            // FilesystemSnapshotPathMapper object
            FilesystemSnapshotPathMapper mapper = this->snapshotPathMappers.value( driveLetter );
            mapper.addFileToRelativeIncludes( relativeFileName, true);
            this->snapshotPathMappers.insert (driveLetter, mapper );
        } else
        {
            // Insert the partition name with a corresponding
            // FilesystemSnapshotPathMapper object to the SnapshotMapper
            QHash<QString,bool> empty;
            FilesystemSnapshotPathMapper mapper(driveLetter, empty);
            mapper.addFileToRelativeIncludes( relativeFileName, true);
            this->snapshotPathMappers.insert( driveLetter, mapper );
            qDebug() << "Added a new snapshotmapper for partition" << driveLetter << "which is:" << this->snapshotPathMappers.value( driveLetter).getRelativeIncludes();
        }
    }

    //qDebug() << "SnapshotPathMappers now are:" << this->snapshotPathMappers;

    // Create temporary VSS_ID
    VSS_ID tmp_snapshot_set_id;

    // Go through all partitions from the SnapshotMapper and add them to the
    // Snapshot set
    foreach ( QString partition , this->snapshotPathMappers.keys() )
    {
        QString partition_name = partition;
	partition_name.append(":\\");
	qDebug() << "Adding partition" << partition_name << "to snapshot set";

        // Convert the partition name to a wchar array
        WCHAR win_partition[4] = {0};
        partition_name.toWCharArray(win_partition);

        // Add the partition to the snapshot set
        HRESULT result = this->pBackup->AddToSnapshotSet(win_partition, GUID_NULL, &tmp_snapshot_set_id);

        // Check if the operation succeeded
        if (result != S_OK)
        {
            QString error = QString("Adding %s to snapshot set failed with error: 0x%08lx\n").arg(partition_name).arg(result);
            emit sendFilesAddedToSnapshot( SNAPSHOT_CANNOT_ADD_PARTITION_TO_SNAPSHOT_SET );
            return;
        }

        // Add the partition name and the snapshot set id the the hash map
        qDebug() << "Adding partition" << partition << "to the snapshot_set_ids";
        this->snapshot_set_ids.insert(partition, tmp_snapshot_set_id);
    }

    emit sendFilesAddedToSnapshot( SNAPSHOT_SUCCESS );
}

void ShadowCopy::takeSnapshot()
{
    // Set the backup state
    HRESULT result = this->pBackup->SetBackupState(this->SC_SNAPSHOT_SELECT_COMPONENTS, this->SC_SNAPSHOT_BOOTABLE_STATE, this->SC_SNAPSHOT_TYPE);

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Setting backup state failed with error: 0x%08lx\n").arg(result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_SET_SNAPSHOT_STATE );
        return;
    }

    // Tell everyone to prepare for the backup
    result = this->pBackup->PrepareForBackup(&(this->pPrepare));

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Preparing for backup failed with error: 0x%08lx\n").arg(result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_PREPARE_FOR_BACKUP );
        return;
    }

    // Wait for everyone to be ready
    result = this->pPrepare->Wait();

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Waiting for preparing for backup failed with error: 0x%08lx\n").arg(result);
        emit sendSnapshotTaken( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    // And create the shadow copy
    result = this->pBackup->DoSnapshotSet(&(this->pDoShadowCopy));

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Creating shadow copy snapshot failed with error: 0x%08lx\n").arg(result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_CREATE_SNAPSHOT );
        return;
    }

    // Wait until the shadow copy is created
    result = this->pDoShadowCopy->Wait();

    // Check if the operation succeeded
    if (result != S_OK)
    {
        QString error = QString("Waiting for shadow copy to finish failed with error: 0x%08lx\n").arg(result);
        emit sendSnapshotTaken( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    // Go through all snapshots and get the according properties
    foreach ( QString partition, this->snapshot_set_ids.keys() )
    {
        VSS_SNAPSHOT_PROP tmp_snapshot_prop;

        // Get the and set them to the snapshot properties field
        result = this->pBackup->GetSnapshotProperties(this->snapshot_set_ids.value( partition) , &tmp_snapshot_prop);

        // Check if the operation succeeded
        if (result != S_OK)
        {
            QString error = QString("Getting snapshot properties failed with error: 0x%08lx\n").arg(result);
            emit sendSnapshotTaken( SNAPSHOT_CANNOT_GET_SNAPSHOT_PROPERTIES );
            return;
        }

        // Store the snapshot properties according to the partition name in the
        // FilesystemSnapshotPathMapper object
        QString snapshotPath = wCharArrayToQString(tmp_snapshot_prop.m_pwszSnapshotDeviceObject);

        // Remove the frist 22 characters: \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy7   ->   HarddiskVolumeShadowCopy7
        QStringRef harddiskVolumeShadowCopy = snapshotPath.midRef(22);
        QString snapshotPathForCygwin = "/proc/sys/device/" + harddiskVolumeShadowCopy.toString() + "/";

        FilesystemSnapshotPathMapper mapper = this->snapshotPathMappers.value( partition );
        mapper.setSnapshotPath( snapshotPathForCygwin );
        this->snapshotPathMappers.insert( partition, mapper);

        // As the mounting takes some time, delay the execution of the next
        // loop itereation
        QWaitCondition waitCondition;
        QMutex mutex;
        waitCondition.wait(&mutex, 500);
    }

    emit sendSnapshotTaken( SNAPSHOT_SUCCESS );
}

void ShadowCopy::cleanupSnapshot()
{
    // Go through all snapshot and delete the shadow copy itself.

    if ( this->snapshot_set_ids.isEmpty() )
    {
        // If this hash is empty, nothing has been done yet. So 
        // just send the signal that cleanup was successfull 
        emit sendSnapshotCleandUp( SNAPSHOT_SUCCESS );
        return;
    } 

    // Finally remove the shadow copy itself

    // Get the snapshot properties for the given partition
    VSS_SNAPSHOT_PROP tmp_snapshot_prop;

    // Get the and set them to the snapshot properties field
    QString part = this->snapshot_set_ids.keys().first();
    HRESULT result = this->pBackup->GetSnapshotProperties(this->snapshot_set_ids.value( part ), &tmp_snapshot_prop);

    // Check if the operation succeeded
    if (result == S_OK)
    {
        // Get the ID from the shadow copy
        VSS_ID shadowCopyID = tmp_snapshot_prop.m_SnapshotSetId;

        // And delete it (as the copies are non-persistent, we do not care about
        // errors in the deletion process)
        this->pBackup->DeleteSnapshots( shadowCopyID, VSS_OBJECT_SNAPSHOT_SET,
                                         true, nullptr, nullptr);
    }

    // Send that the cleanup is done
    emit sendSnapshotCleandUp( SNAPSHOT_SUCCESS );
}

const SnapshotMapper& ShadowCopy::getSnapshotPathMappers()
{
    return this->snapshotPathMappers;
}

void ShadowCopy::setSnapshotPathMappers(
        const SnapshotMapper& snapshotPathMappers)
{
    this->snapshotPathMappers = snapshotPathMappers;
}

QString ShadowCopy::wCharArrayToQString( WCHAR* string)
{
    return QString::fromWCharArray( string, wcslen(string) );
}
