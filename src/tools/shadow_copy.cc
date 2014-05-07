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
#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QWaitCondition>
#include <QMutex>

const QString ShadowCopy::MOUNT_DIRECTORY = "C:\\"; // TODO change this to something more appropriate like tmp folder or the sepiola app data dir
const QString ShadowCopy::MOUNT_PREFIX = "mount_shadow_copy_";

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
    this->vssapiBase = LoadLibrary("vssapi.dll");

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
        qDebug() << "Adding file" << file << "to the snapshot mapper";

        // Get the driveletter of the current file
        QString driveLetter = getDriveLetterByFile( file );

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
        this->result = this->pBackup->AddToSnapshotSet(win_partition, GUID_NULL, &tmp_snapshot_set_id);

        // Check if the operation succeeded
        if (this->result != S_OK)
        {
            QString error = QString("Adding %s to snapshot set failed with error: 0x%08lx\n").arg(partition_name).arg(this->result);
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
    foreach ( QString partition, this->snapshot_set_ids.keys() )
    {
        VSS_SNAPSHOT_PROP tmp_snapshot_prop;

        // Get the and set them to the snapshot properties field
        this->result = this->pBackup->GetSnapshotProperties(this->snapshot_set_ids.value( partition) , &tmp_snapshot_prop);

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
	
        QString linkname = MOUNT_DIRECTORY;
        linkname.append( MOUNT_PREFIX );
        linkname.append(partition);
        snapshotPath.append("\\");

        QString args = linkname;
        args.append(" ").append(snapshotPath);
        const char* char_args = args.toUtf8().constData();

        HMODULE lib;
        CreateSymbolicLinkProc CreateSymbolicLink_func;
        DWORD flags = 1;

        lib = LoadLibrary("kernel32");
        CreateSymbolicLink_func =
            (CreateSymbolicLinkProc)GetProcAddress(lib,"CreateSymbolicLinkW");

        QString fullLinkname = linkname;
        fullLinkname.prepend("\\\\?\\");

        LPCSTR link = (LPCSTR) fullLinkname.utf16();
        LPCSTR target = (LPCSTR) snapshotPath.utf16();

        if (CreateSymbolicLink_func == NULL)
        {
            qDebug() << "CreateSymbolicLinkW not available";
            emit sendSnapshotTaken( 4 );
        } else
        {
            if ((*CreateSymbolicLink_func)(link, target, flags) == 0)
            {
                qDebug() <<  "CreateSymbolicLink failed" << GetLastError();
                emit sendSnapshotTaken( SNAPSHOT_ASYNC_WAIT_FAILED );
                return;
            }
        }

        FilesystemSnapshotPathMapper mapper = this->snapshotPathMappers.value( partition );
        mapper.setSnapshotPath( linkname );
        this->snapshotPathMappers.insert( partition, mapper);

        // As the mounting takes some time, delay the execution of the next
        // loop itereation
        QWaitCondition waitCondition;
        QMutex mutex;
        waitCondition.wait(&mutex, 500);

    }

    emit sendSnapshotTaken( SNAPSHOT_SUCCESS );
}


QString ShadowCopy::getDriveLetterByFile( const QString filename )
{
    // The filename will be something like <LETTER>:\path\to\file so get the
    // <LETTER>:
    QRegExp regex("^\\w:\\/");

    // Get the first occurrence of the regex in the filename
    int pos = regex.indexIn( filename );

    QString letter;
    if ( pos > -1 )
    {
        letter = regex.cap(0).left(1);
    } else
    {
        // TODO: What if no drive letter was found?
        qDebug() << filename << "does not math ^\\w:\\/";
    }

    // Return the drive letter
    return letter;

}

void ShadowCopy::cleanupSnapshot()
{
    // Go through all snapshot and delete the shadow copy itself and the symlink
    // to it
    foreach ( QString partition, this->snapshot_set_ids.keys() )
    {
        // Get the symlink name for the given partition
        QString linkname = this->snapshotPathMappers.value( partition ).getSnapshotPath();
        linkname.prepend("\\\\?\\");

        HMODULE lib;
        RemoveDirectoryProc RemoveDirectory_func;

        lib = LoadLibrary("kernel32");
        RemoveDirectory_func =
            (RemoveDirectoryProc)GetProcAddress(lib,"RemoveDirectoryW");

        LPCSTR link = (LPCSTR) linkname.utf16();

        (*RemoveDirectory_func)(link);

        // Simply remove the symlink
        //QDir::remove( linkname );
    }

    // Finally remove the shadow copy itself

    // Get the snapshot properties for the given partition
    VSS_SNAPSHOT_PROP tmp_snapshot_prop;

    // Get the and set them to the snapshot properties field
    QString part = this->snapshot_set_ids.keys().first();
    this->result = this->pBackup->GetSnapshotProperties(this->snapshot_set_ids.value( part ), &tmp_snapshot_prop);

    // Check if the operation succeeded
    if (this->result == S_OK)
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

void ShadowCopy::checkCleanup()
{
    // Check in the MOUNT_DIRECTORY if there are any links with MOUNT_PREFIX
    QStringList nameFilter(MOUNT_PREFIX + "*");
    QDir directory(MOUNT_DIRECTORY);
    QStringList oldShadowCopyMounts = directory.entryList(nameFilter);

    // If yes, remove them
    foreach( QString oldMount, oldShadowCopyMounts )
    {
        // Take the linkname and prepend the necessary Windows UNC path
        QString linkname = oldMount;
        linkname.prepend("\\\\?\\");

        HMODULE lib;
        RemoveDirectoryProc RemoveDirectory_func;

        lib = LoadLibrary("kernel32");
        RemoveDirectory_func =
            (RemoveDirectoryProc)GetProcAddress(lib,"RemoveDirectoryW");

        LPCSTR link = (LPCSTR) linkname.utf16();

        (*RemoveDirectory_func)(link);
    }

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
