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

#include <QMessageBox>

#include "exception/process_exception.hh"
#include "model/main_model.hh"
#include "model/restore_thread.hh"
#include "settings/settings.hh"
#include "tools/tool_factory.hh"
#include "utils/file_system_utils.hh"


RestoreThread::RestoreThread( const QString& backupName, const QString& destination )
{
	this->isCustomRestore = false;
	this->backupName = backupName;
	this->destination = destination;
	init();
}

RestoreThread::RestoreThread( const QString& backupName, const QStringList& items, const QString& destination )
{
	this->isCustomRestore = true;
	this->backupName = backupName;
	this->items = items;
	this->destination = destination;
	init();
}

RestoreThread::~RestoreThread()
{
	qDebug() << "RestoreThread::~RestoreThread()";
	QObject::disconnect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
							 this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::disconnect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
							 this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::disconnect( this, SIGNAL( abort() ),
							 rsync.get(), SLOT( abort() ) );
}

void RestoreThread::init()
{
	isAborted = false;
	rsync = ToolFactory::getRsyncImpl();
	QObject::connect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
						this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::connect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
						this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::connect( this, SIGNAL( abort() ),
						rsync.get(), SLOT( abort() ) );
	connect( this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void RestoreThread::run()
{
	try
	{
		emit appendInfoMessage( tr( "Downloading files and/or directories ..." ) );
		QStringList downloadedItems;
		if ( isCustomRestore )
		{
			downloadedItems = rsync->downloadCustomBackup( backupName, items, destination );
		}
		else
		{
			downloadedItems = rsync->downloadFullBackup( backupName, destination );
		}
		checkAbortState();
		emit appendInfoMessage( tr( "Applying Metadata" ) );
		applyMetadata( backupName, downloadedItems, destination );
		checkAbortState();
		emit appendInfoMessage( tr( "Restore done." ) );
	}
	catch ( ProcessException e )
	{
		emit appendInfoMessage( tr( "Restore failed." ) );
		emit appendErrorMessage( e.what() );
	}
	catch ( AbortException e )
	{
		emit appendInfoMessage( tr( "Backup aborted." ) );
		emit appendErrorMessage( e.what() );
	}

	emit finishProgressDialog();
}

void RestoreThread::applyMetadata( const QString& backupName, const QStringList& downloadedItems, const QString& downloadDestination )
{
	if ( downloadedItems.size() > 0 )
	{
		auto_ptr< AbstractMetadata > metadata = ToolFactory::getMetadataImpl();
		QObject::connect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
							this, SIGNAL( appendInfoMessage( const QString& ) ) );
		QObject::connect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
							this, SIGNAL( appendErrorMessage( const QString& ) ) );
		QFileInfo metadataFile = rsync->downloadMetadata( backupName, Settings::getInstance()->getApplicationDataDir() );
		metadata->setMetadata( metadataFile, downloadedItems, downloadDestination );
		FileSystemUtils::removeFile( metadataFile );
		QObject::disconnect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
							this, SIGNAL( appendInfoMessage( const QString& ) ) );
		QObject::disconnect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
							this, SIGNAL( appendErrorMessage( const QString& ) ) );
	}
}

void RestoreThread::checkAbortState() throw ( AbortException )
{
	if ( isRunning() && isAborted )
	{
		throw AbortException( tr( "Restore has been aborted" ) );
	}
}

bool RestoreThread::abortRestoreProcess()
{
	qDebug() << "RestoreThread::abortRestoreProcess()";
	/*int answer = QMessageBox::question ( this, tr ( "Cancel restore process?" ),
																			 tr ( "The restore process has not been finished.\nThis will cancel the restore process.\nAre you sure you want to cancel?" ),
																			 QMessageBox::Yes | QMessageBox::No ); */
	int answer = QMessageBox::Yes; // TODO: make working QMessageBox::question - request
	switch ( answer )
	{
		case QMessageBox::Yes:
			isAborted = true;
			emit abort();
			return true;
		case QMessageBox::No:
			return false;
	}
	return false;
}
