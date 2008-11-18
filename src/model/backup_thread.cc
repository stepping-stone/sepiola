/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
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

#include <QDebug>
#include <QDir>
#include <QPair>
#include <QSet>

#include "exception/process_exception.hh"
#include "exception/abort_exception.hh"
#include "model/backup_thread.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "tools/tool_factory.hh"
#include "tools/rsync.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/unicode_text_stream.hh"

BackupThread::BackupThread( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const bool& setDeleteFlag )
{
	isAborted = false;
	this->items = items;
	this->includePatternList = includePatternList;
	this->excludePatternList = excludePatternList;
	this->setDeleteFlag = setDeleteFlag;

	rsync = ToolFactory::getRsyncImpl();
	QObject::connect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
						this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::connect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
						this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::connect( this, SIGNAL( abort() ),
						rsync.get(), SLOT( abort() ) );
	connect( this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

BackupThread::~BackupThread()
{
	qDebug() << "BackupThread::~BackupThread()";
	QObject::disconnect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
							 this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::disconnect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
							 this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::disconnect( this, SIGNAL( abort() ),
							 rsync.get(), SLOT( abort() ) );
}

void BackupThread::startInCurrentThread()
{
	run();
}

void BackupThread::run()
{
	qDebug() << "BackupThread::run()";
	bool failed = false;
	try
	{
		checkAbortState();
		Settings* settings = Settings::getInstance();
		emit appendInfoMessage( tr( "Creating/validating server directories" ) );
		prepareServerDirectories();
		checkAbortState();

		emit appendInfoMessage( tr( "Uploading files and directories" ) );
		QString source = "/";
		QString destination = settings->getUserName() + "@" + settings->getServerName() + ":" + settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/";

		QString errors;
		QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems = rsync->upload( items, source, destination, includePatternList, excludePatternList, setDeleteFlag, false, &errors );
		if ( errors != "" )
		{
			failed = true;
			emit appendErrorMessage( errors );
		}
		checkAbortState();

		if ( processedItems.size() > 0 )
		{
			QString metaDataDir = settings->getUserName() + "@" + settings->getServerName() + ":" +
				settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getMetaFolderName();
			//auto_ptr< AbstractSsh > ssh = ToolFactory::getSshImpl();

			// backup content file list
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getBackupContentFileName() );
			emit appendInfoMessage( tr( "Preparing meta data" ) );
			emit appendInfoMessage( tr( "Getting backup content meta data" ) );
			QFileInfo currentBackupContentFile = rsync->downloadCurrentBackupContentFile( settings->getApplicationDataDir(), false );
			checkAbortState();
			updateBackupContentFile( currentBackupContentFile, processedItems );
			emit appendInfoMessage( tr( "Uploading backup content meta data" ) );
			//ssh->uploadToMetaFolder( currentBackupContentFile, false );
			rsync->upload( currentBackupContentFile, metaDataDir, true );
			//FileSystemUtils::removeFile( currentBackupContentFile );
			checkAbortState();

			// backup time file
			emit appendInfoMessage( tr( "Uploading backup info data" ) );
			QString currentBackupTimeFile = createCurrentBackupTimeFile();
			//ssh->uploadToMetaFolder( currentBackupTimeFile, false );
			rsync->upload( currentBackupTimeFile, metaDataDir, true );
			FileSystemUtils::removeFile( currentBackupTimeFile );
			checkAbortState();

			// permission file
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getMetadataFileName() );
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getTempMetadataFileName() );
			emit appendInfoMessage( tr( "Getting permission meta data" ) );
			auto_ptr< AbstractMetadata > metadata = ToolFactory::getMetadataImpl();
			QObject::connect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
								this, SIGNAL( appendInfoMessage( const QString& ) ) );
			QObject::connect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
								this, SIGNAL( appendErrorMessage( const QString& ) ) );
			QFileInfo currentMetadataFileName = rsync->downloadCurrentMetadata( settings->getApplicationDataDir(), false );
			checkAbortState();
			QFileInfo newMetadataFileName = metadata->getMetadata( processedItems );
			checkAbortState();
			metadata->mergeMetadata( newMetadataFileName, currentMetadataFileName, processedItems );
			checkAbortState();
			emit appendInfoMessage( tr( "Uploading permission meta data" ) );
			//ssh->uploadToMetaFolder( currentMetadataFileName, false );
			rsync->upload( currentMetadataFileName, metaDataDir, true );
			checkAbortState();
			//FileSystemUtils::removeFile( currentMetadataFileName );
			//FileSystemUtils::removeFile( newMetadataFileName );
			QObject::disconnect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
								this, SIGNAL( appendInfoMessage( const QString& ) ) );
			QObject::disconnect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
								this, SIGNAL( appendErrorMessage( const QString& ) ) );
			if( !failed )
			{
				emit appendInfoMessage( tr( "Backup succeeded." ) );
			}
		}
	}
	catch ( ProcessException e )
	{
		failed = true;
		emit appendErrorMessage( e.what() );
	}
	catch ( AbortException e )
	{
		emit appendInfoMessage( tr( "Backup aborted." ) );
		emit appendErrorMessage( e.what() );
	}
	if ( failed )
	{
		emit appendInfoMessage( tr( "Backup failed." ) );
	}
	emit finishProgressDialog();
}

void BackupThread::checkAbortState() throw ( AbortException )
{
	if ( isRunning() && isAborted )
	{
		throw AbortException( tr( "Backup has been aborted" ) );
	}
}


void BackupThread::prepareServerDirectories()
{
	qDebug() << "BackupThread::prepareServerDirectories()";
	// create local directories and upload them
	Settings* settings = Settings::getInstance();
	QString source = settings->getApplicationDataDir();
	QString destination = settings->getUserName() + "@" + settings->getServerName() + ":" + settings->getBackupRootFolder();
	QDir workingDir( settings->getApplicationDataDir() );
	QString backupPrefixFolder = settings->getBackupPrefix();
	workingDir.mkdir( backupPrefixFolder );
	QDir backupPrefixDir( settings->getApplicationDataDir() + settings->getBackupPrefix() );
	checkAbortState();

	//AbstractRsync* rsync = ToolFactory::getRsyncImpl();
	QString errors;
	rsync->upload( QStringList( backupPrefixFolder ), source, destination, QStringList(), QStringList(), false, false, &errors );
	checkAbortState();
	backupPrefixDir.mkdir( settings->getMetaFolderName() );
	backupPrefixDir.mkdir( settings->getBackupFolderName() );
	rsync->upload( QStringList( backupPrefixFolder ), source, destination, QStringList(), QStringList(), false, false, &errors );
	checkAbortState();
	//delete rsync;

	// delete local directories
	backupPrefixDir.rmdir( settings->getMetaFolderName() );
	backupPrefixDir.rmdir( settings->getBackupFolderName() );
	workingDir.rmdir( backupPrefixFolder );
}

void BackupThread::updateBackupContentFile( const QFileInfo& backupContentFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& backupList )
{
	QSet<QString> existingItems;
	QFile backupContentFile( backupContentFileName.absoluteFilePath() );
	if ( backupContentFile.open( QIODevice::ReadOnly ) )
	{
		UnicodeTextStream inputStream( &backupContentFile );
		while ( !inputStream.atEnd() )
		{
			existingItems << inputStream.readLine();
			checkAbortState();
		}
		backupContentFile.close();
		backupContentFile.remove();
	}

	for( int i=0; i<backupList.size(); i++ )
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>  backupedItem = backupList.at( i );
		FileSystemUtils::convertToServerPath( &backupedItem.first );

		switch( backupedItem.second )
		{
			case AbstractRsync::DELETED:
				existingItems.remove( backupedItem.first );
				break;
			default:
				existingItems << backupedItem.first;
				break;
		}
		checkAbortState();
	}
	//idea: update the file directly instead of re-writinig the whole content again
	if ( !backupContentFile.open( QIODevice::WriteOnly ) )
	{
		qWarning() << "Can not write backup content file " << backupContentFileName.absoluteFilePath();
		return;
	}
	UnicodeTextStream outputStream( &backupContentFile );
	outputStream.setCodec( "UTF-8" );
	foreach( QString item, existingItems )
	{
		outputStream << item << endl;
		checkAbortState();
	}
	backupContentFile.close();
}

QString BackupThread::createCurrentBackupTimeFile()
{
	Settings* settings = Settings::getInstance();
	QString fileName = settings->getApplicationDataDir() + settings->getBackupTimeFileName();
	QFile file( fileName );
	if ( !file.open( QIODevice::WriteOnly ) )
	{
		emit showCriticalMessageBox( tr( "Can not create a backup info file" ) );
		return fileName;
	}
	UnicodeTextStream out( &file );
	out << MainModel::BACKUP_TIME_TODAY ;
	file.close();
	return fileName;
}

void BackupThread::abortBackupProcess()
{
	qDebug() << "BackupThread::abortBackupProcess()";
	isAborted = true;
	emit abort();
}
