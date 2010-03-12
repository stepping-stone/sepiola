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

#include "exception/process_exception.hh"
#include "model/main_model.hh"
#include "model/restore_thread.hh"
#include "settings/settings.hh"
#include "tools/tool_factory.hh"
#include "utils/file_system_utils.hh"


/**
 * deprecated, fullRestore is done by passing selectionRule("/",true)
 */
/* RestoreThread::RestoreThread( const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	this->isCustomRestore = false;
	this->backup_prefix = backup_prefix;
	this->backupName = backupName;
	this->destination = destination;
	init();
} */

/**
 * deprecated
 */
/* RestoreThread::RestoreThread( const QString& backup_prefix, const QString& backupName, const QStringList& items, const QString& destination )
{
	this->isCustomRestore = true;
	this->backup_prefix = backup_prefix;
	this->backupName = backupName;
	this->items = items;
	this->destination = destination;
	init();
} */

RestoreThread::RestoreThread( const QString& backup_prefix, const QString& backupName, const BackupSelectionHash& selectionRules, const QString& destination )
{
	this->isCustomRestore = true;
	this->backup_prefix = backup_prefix;
	this->backupName = backupName;
	this->selectionRules = selectionRules;
	this->destination = destination;
	init();
}

RestoreThread::~RestoreThread()
{
	qDebug() << "RestoreThread::~RestoreThread()";
	QObject::disconnect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
						 this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::disconnect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
						 this, SIGNAL( errorSignal( const QString& ) ) );
	QObject::disconnect( this, SIGNAL( abort() ),
						 rsync.get(), SLOT( abort() ) );
	//QObject::disconnect( rsync.get(), SIGNAL( trafficInfoSignal( const QString&, float, quint64, quint64 ) ),
	//					 this, SLOT( rsyncUploadProgressHandler( const QString&, float, quint64, quint64 ) ) );
}

void RestoreThread::init()
{
	isAborted = false;
	rsync = ToolFactory::getRsyncImpl();
	QObject::connect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
					  this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::connect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
					  this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::connect( this, SIGNAL( abort() ),
					  rsync.get(), SLOT( abort() ) );
	//QObject::connect( rsync.get(), SIGNAL( trafficInfoSignal( const QString&, float, quint64, quint64 ) ),
	//				  this, SLOT( rsyncUploadProgressHandler( const QString&, float, quint64, quint64 ) ) );
	// TODO: this is copied from BackupThread -> adapt as needed, as well as the above disconnection
	connect( this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void RestoreThread::run()
{
	try
	{
		emit infoSignal( tr( "Downloading files and/or directories ..." ) );
		QStringList downloadedItems;
		if ( isCustomRestore )
		{
			downloadedItems = rsync->downloadCustomBackup( backup_prefix, backupName, selectionRules, destination );
		}
		else
		{
			downloadedItems = rsync->downloadFullBackup( backup_prefix, backupName, destination );
		}
		checkAbortState();
		emit infoSignal( tr( "Applying Metadata" ) );
		applyMetadata( backup_prefix, backupName, downloadedItems, destination );
		checkAbortState();
		emit infoSignal( tr( "Restore done." ) );
	}
	catch ( ProcessException e )
	{
		emit infoSignal( tr( "Restore failed." ) );
		emit errorSignal( e.what() );
	}
	catch ( AbortException e )
	{
		emit infoSignal( tr( "Backup aborted." ) );
		emit errorSignal( e.what() );
	}

	emit finishProgressDialog();
}

void RestoreThread::applyMetadata( const QString& backup_prefix, const QString& backupName, const QStringList& downloadedItems, const QString& downloadDestination )
{
	if ( downloadedItems.size() > 0 )
	{
		auto_ptr< AbstractMetadata > metadata = ToolFactory::getMetadataImpl();
		QObject::connect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
						  this, SIGNAL( infoSignal( const QString& ) ) );
		QObject::connect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
						  this, SIGNAL( errorSignal( const QString& ) ) );
		QFileInfo metadataFile = rsync->downloadMetadata( backup_prefix, backupName, Settings::getInstance()->getApplicationDataDir() );
		metadata->setMetadata( metadataFile, downloadedItems, downloadDestination );
		FileSystemUtils::removeFile( metadataFile );
		QObject::disconnect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
							 this, SIGNAL( infoSignal( const QString& ) ) );
		QObject::disconnect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
							 this, SIGNAL( infoSignal( const QString& ) ) );
	}
}

void RestoreThread::checkAbortState() throw ( AbortException )
{
	if ( isRunning() && isAborted )
	{
		throw AbortException( tr( "Restore has been aborted" ) );
	}
}

void RestoreThread::abortRestoreProcess()
{
	qDebug() << "RestoreThread::abortRestoreProcess()";
	isAborted = true;
	emit abort();
}
