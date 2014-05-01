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

#include <QString>
#include <QDebug>
#include <QRegExp>

/**
 * The dummy snapshot class provides an empty snapshot method
 * @author Pat Kläy, pat.klaey@stepping-stone.ch
 */
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
    this->vssapiBase = LoadLibrary(L"vssapi.dll");

    // Get the
    CreateVssBackupComponentsInternal_I = (_CreateVssBackupComponentsInternal)GetProcAddress(this->vssapiBase, "CreateVssBackupComponentsInternal");

    // Create the shadow copy backup component (VSSBackupComponent)
    this->result = CreateVssBackupComponentsInternal_I(&(this->pBackup));

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Cannot create VSSBackupComponent, operation failed with error: 0x%08lx").arg(this->result);
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

    this->result = this->pBackup->InitializeForBackup();

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Initialize for backup failed with error: = 0x%08lx").arg(this->result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_INITIALIZE_BACKUP );
        return;
    }

    // Set the context, we want an non-persistant backup which involves writers
    this->result = this->pBackup->SetContext(this->SC_SNAPSHOT_CONTEXT);

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Setting backup context to %i failed with error: 0x%08lx").arg(this->SC_SNAPSHOT_CONTEXT).arg(this->result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_CANNOT_SET_BACKUP_CONTEXT );
        return;
    }

    // Tell the writers to gather metadata
    this->result = this->pBackup->GatherWriterMetadata(&(this->pAsync));

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Writers gathering metadata failed with error: 0x%08lx").arg(this->result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_WRITER_GATHERING_METADATA_FAILED );
        return;
    }

    // Wait until all writers collected the metadata
    this->pAsync->Wait();

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Waiting for writers collecting metadata failed with error: 0x%08lx").arg(this->result);
        qCritical() << error;
        emit sendSnapshotInitialized( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    VSS_ID tmp_snapshot_set_id;

    // Start the snapshot set
    this->result = this->pBackup->StartSnapshotSet(&tmp_snapshot_set_id);

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Starting snapshot set failed with error: 0x%08lx").arg(this->result);
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
        // Get the driveletter of the current file
        QString driveLetter = getDriveLetterByFile( file );

        // Get the relative filename
        QString relativeFileName = file;
        relativeFileName.replace( driveLetter, QString(""));

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
        }
    }

    // Create temporary VSS_ID
    VSS_ID tmp_snapshot_set_id;

    // Go through all partitions from the SnapshotMapper and add them to the
    // Snapshot set
    foreach ( QString partition_name , this->snapshotPathMappers.keys() )
    {
        // Convert the partition name to a wchar array
        WCHAR partition[4] = {0};
        partition_name.toWCharArray(partition);

        // Add the partition to the snapshot set
        this->result = this->pBackup->AddToSnapshotSet(partition, GUID_NULL, &tmp_snapshot_set_id);

        // Check if the operation succeeded
        if (this->result != S_OK)
        {
            QString error = QString("Adding %s to snapshot set failed with error: 0x%08lx\n").arg(partition_name).arg(this->result);
            emit sendFilesAddedToSnapshot( SNAPSHOT_CANNOT_ADD_PARTITION_TO_SNAPSHOT_SET );
            return;
        }

        // Add the partition name and the snapshot set id the the hash map
        this->snapshot_set_ids[partition] = tmp_snapshot_set_id;
    }

    emit sendFilesAddedToSnapshot( SNAPSHOT_SUCCESS );
}

void ShadowCopy::takeSnapshot()
{
    // Set the backup state
    this->result = this->pBackup->SetBackupState(this->SC_SNAPSHOT_SELECT_COMPONENTS, this->SC_SNAPSHOT_BOOTABLE_STATE, this->SC_SNAPSHOT_TYPE);

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Setting backup state failed with error: 0x%08lx\n").arg(this->result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_SET_SNAPSHOT_STATE );
        return;
    }

    // Tell everyone to prepare for the backup
    this->result = this->pBackup->PrepareForBackup(&(this->pPrepare));

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Preparing for backup failed with error: 0x%08lx\n").arg(this->result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_PREPARE_FOR_BACKUP );
        return;
    }

    // Wait for everyone to be ready
    this->result = this->pPrepare->Wait();

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Waiting for preparing for backup failed with error: 0x%08lx\n").arg(this->result);
        emit sendSnapshotTaken( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    // And create the shadow copy
    this->result = this->pBackup->DoSnapshotSet(&(this->pDoShadowCopy));

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Creating shadow copy snapshot failed with error: 0x%08lx\n").arg(this->result);
        emit sendSnapshotTaken( SNAPSHOT_CANNOT_CREATE_SNAPSHOT );
        return;
    }

    // Wait until the shadow copy is created
    this->result = this->pDoShadowCopy->Wait();

    // Check if the operation succeeded
    if (this->result != S_OK)
    {
        QString error = QString("Waiting for shadow copy to finish failed with error: 0x%08lx\n").arg(this->result);
        emit sendSnapshotTaken( SNAPSHOT_ASYNC_WAIT_FAILED );
        return;
    }

    // Go through all snapshots and get the according properties
    for (auto &tmp : this->snapshot_set_ids)
    {
        VSS_SNAPSHOT_PROP tmp_snapshot_prop;

        // Get the and set them to the snapshot properties field
        this->result = this->pBackup->GetSnapshotProperties(tmp.second, &tmp_snapshot_prop);

        // Check if the operation succeeded
        if (this->result != S_OK)
        {
            QString error = QString("Getting snapshot properties failed with error: 0x%08lx\n").arg(this->result);
            emit sendSnapshotTaken( SNAPSHOT_CANNOT_GET_SNAPSHOT_PROPERTIES );
            return;
        }

        // Store the snapshot properties according to the partition name in the
        // FilesystemSnapshotPathMapper object
        QString snapshotPath = wCharArrayToQString(tmp_snapshot_prop.m_pwszSnapshotDeviceObject);
        QString partition = wCharArrayToQString(tmp.first);
        FilesystemSnapshotPathMapper mapper = this->snapshotPathMappers.value( partition );
        mapper.setSnapshotPath( snapshotPath );
        this->snapshotPathMappers.insert( partition, mapper);
    }

    emit sendSnapshotTaken( SNAPSHOT_SUCCESS );
}


QString ShadowCopy::getDriveLetterByFile( const QString filename )
{
    // The filename will be something like <LETTER>:\path\to\file so get the
    // <LETTER>:
    QRegExp regex("^\w:\\");

    // Get the first occurrence of the regex in the filename
    int pos = regex.indexIn( filename );

    QString letter;
    if ( pos > -1 )
    {
        letter = regex.cap(0);
    } else
    {
        // TODO: What if no drive letter was found?
    }

    // Return the drive letter
    return letter;

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
    return QString::fromWCharArray( string, sizeof(string) );
}
