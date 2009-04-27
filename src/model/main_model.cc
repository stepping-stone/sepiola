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

#include "cli/cli_manager.hh"
#include "exception/login_exception.hh"
#include "model/backup_thread.hh"
#include "model/backup_task.hh"
#include "model/dir_tree_item.hh"
#include "model/main_model.hh"
#include "model/remote_dir_model.hh"
#include "model/restore_thread.hh"
#include "tools/abstract_rsync.hh"
#include "tools/abstract_metadata.hh"
#include "tools/rsync.hh"
#include "tools/posix_acl.hh"
#include "tools/tool_factory.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/unicode_text_stream.hh"

const QString MainModel::BACKUP_TIME_FORMAT = "yyyy-MM-dd HH:mm";
const QString MainModel::BACKUP_TIME_TODAY = "TODAY";

MainModel::MainModel() : localDirModel(0)
{
}

MainModel::~MainModel()
{
	delete localDirModel;
}

void MainModel::keepSettings()
{
	Settings* settings = Settings::getInstance();
	auto_ptr< AbstractScheduler > scheduler = ToolFactory::getSchedulerImpl();
	QObject::connect( scheduler.get(), SIGNAL( askForPassword( const QString&, bool, int*, const QString& ) ),
						this, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ) );
	QObject::connect( scheduler.get(), SIGNAL( infoSignal( const QString& ) ),
						this, SLOT( infoDialog( const QString& ) ) );
	QObject::connect( scheduler.get(), SIGNAL( errorSignal( const QString& ) ),
						this, SLOT( errorDialog( const QString& ) ) );

	scheduler->updateExistingTask( settings->getThisApplicationFullPathExecutable(), CliManager::SCHEDULE_ARGUMENT );
	settings->keepSettings();

	QObject::disconnect( scheduler.get(), SIGNAL( askForPassword( const QString&, bool, int*, const QString& ) ),
						this, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ) );
	QObject::disconnect( scheduler.get(), SIGNAL( infoSignal( const QString& ) ),
						this, SLOT( infoDialog( const QString& ) ) );
	QObject::disconnect( scheduler.get(), SIGNAL( errorSignal( const QString& ) ),
						this, SLOT( errorDialog( const QString& ) ) );
}

void MainModel::deleteSettings()
{
	auto_ptr< AbstractScheduler > scheduler = ToolFactory::getSchedulerImpl();
	scheduler->deleteExistingTask( Settings::getInstance()->getThisApplicationFullPathExecutable(), CliManager::SCHEDULE_ARGUMENT );
	Settings::getInstance()->deleteSettings();
}

bool MainModel::initConnection()
{
	qDebug() << "MainModel::initConnection()";
	bool isInitialized = false;
	try
	{
		Settings* settings = Settings::getInstance();
		auto_ptr< AbstractSsh > ssh = ToolFactory::getSshImpl();
		QObject::connect( ssh.get(), SIGNAL( infoSignal( const QString& ) ),
							this, SIGNAL( appendInfoMessage( const QString& ) ) );
		QObject::connect( ssh.get(), SIGNAL( errorSignal( const QString& ) ),
							this, SIGNAL( appendErrorMessage( const QString& ) ) );

		emit appendInfoMessage( tr( "Establishing connection ..." ) );
		emit appendInfoMessage( tr( "Validating server's fingerprint ..." ) );
		if ( !ssh->assertCorrectFingerprint() )
		{
			showInformationMessage( tr( "Server fingerprint validation failed" ) );
			return false;
		}
		else
		{
			emit appendInfoMessage( tr( "Server's fingerprint is correct." ) );
		}
		if ( !ssh->loginWithKey() )
		{
			bool isLoggedIn = false;
			isLoginAborted = false;
			while( !isLoggedIn )
			{
				emit askForServerPassword( settings->getServerUserName(), true, 0, "" );
				if ( isLoginAborted )
				{
					emit showCriticalMessageBox( tr( "Key has not been generated." ) );
					return false;
				}
				try {
					if( ssh->generateKeys( settings->getServerPassword() ) )
					{
						isLoggedIn = true;
					}
				} catch ( LoginException e )
				{
					emit appendInfoMessage( tr( "Login failed." ) );
					emit showInformationMessageBox( e.what() );
				}
			}

			emit appendInfoMessage( tr( "Trying to login" ) );
			if ( !ssh->loginWithKey() )
			{
				showInformationMessage( tr( "Can not login to the server. Please review your settings." ) );
			}
			else
			{
				emit appendInfoMessage( tr( "Login succeeded." ) );
				qDebug() << "Second login with key was successful";
				isInitialized = true;
			}
		}
		else
		{
			emit appendInfoMessage( tr( "Login with key was successful" ) );
			isInitialized = true;
		}
		emit appendInfoMessage( tr( "Connection established." ) );
		QObject::disconnect( ssh.get(), SIGNAL( infoSignal( const QString& ) ),
								 this, SIGNAL( appendInfoMessage( const QString& ) ) );
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return isInitialized;
}

void MainModel::backup( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const bool& setDeleteFlag, const bool& startInCurrentThread )
{
	qDebug() << "MainModel::backup( " << items << ", " << includePatternList << ", " << excludePatternList << " )";
	if ( !initConnection() )
	{
		closeProgressDialogSlot();
		return;
	}
	BackupThread* backupThread = new BackupThread( items, includePatternList, excludePatternList, setDeleteFlag );

	QObject::connect( backupThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
						this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
	QObject::connect( backupThread, SIGNAL( appendInfoMessage( const QString& ) ),
						this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::connect( backupThread, SIGNAL( appendErrorMessage( const QString& ) ),
						this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::connect( backupThread, SIGNAL( finishProgressDialog() ),
						this, SIGNAL( finishProgressDialog() ) );
	QObject::connect( this, SIGNAL( abortProcess() ),
						backupThread, SLOT( abortBackupProcess() ) );
	qDebug() << "MainModel::backup: startInCurrentThread=" << startInCurrentThread;
	if ( startInCurrentThread )
	{
		// signals are not connected with QCoreApplication and multiple threads
		// but in CLI mode we do not need an own thread
		backupThread->startInCurrentThread();
		Settings::getInstance()->addLastBackup(BackupTask(QDateTime::currentDateTime(), BackupTask::UNDEFINED));
	}
	else
	{
		backupThread->start();
		//TODO: disconnect signal/slot connections
		BackupTask bkup(QDateTime::currentDateTime(), BackupTask::UNDEFINED);
		Settings::getInstance()->addLastBackup(bkup);
		qDebug() << "MainModel::backup: bkup=" << bkup.toString();
	}
}

bool MainModel::isSchedulingOnStartSupported()
{
	auto_ptr< AbstractScheduler > scheduler = ToolFactory::getSchedulerImpl();
	return scheduler->isSchedulingOnStartSupported();
}

void MainModel::schedule(  const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const ScheduledTask& scheduleRule, const bool& setDeleteFlag )
{
	showProgressDialogSlot( tr( "Verify connection" ) );
	if ( initConnection() )
	{
		closeProgressDialogSlot();
	}
	else
	{
		closeProgressDialogSlot();
		emit showCriticalMessageBox( tr( "Task has not been scheduled, because the connection could not be established" ) );
		return;
	}
	Settings* settings = Settings::getInstance();
	settings->saveBackupItemList( items );
	settings->saveDeleteExtraneousItems( setDeleteFlag );

	auto_ptr< AbstractScheduler > scheduler = ToolFactory::getSchedulerImpl();

	QObject::connect( scheduler.get(), SIGNAL( askForPassword( const QString&, bool, int*, const QString& ) ),
					  this, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ) );
	QObject::connect( scheduler.get(), SIGNAL( infoSignal( const QString& ) ),
					  this, SLOT( infoDialog( const QString& ) ) );
	QObject::connect( scheduler.get(), SIGNAL( errorSignal( const QString& ) ),
					  this, SLOT( errorDialog( const QString& ) ) );

	QString execName = settings->getThisApplicationFullPathExecutable();
	try
	{
		switch (scheduleRule.getType()) {
			case ScheduleRule::AT_WEEKDAYS_AND_TIME:
				scheduler->scheduleTask( settings->getThisApplicationFullPathExecutable(), CliManager::SCHEDULE_ARGUMENT, scheduleRule.getTimeToRun(), scheduleRule.getWeekdaysArray().data() );
				break;
			case ScheduleRule::AFTER_BOOT:
				scheduler->scheduleTask( execName, CliManager::SCHEDULE_ARGUMENT, scheduleRule.getMinutesAfterStartup() );
				break;
			case ScheduleRule::NEVER:
				scheduler->deleteExistingTask( execName, CliManager::SCHEDULE_ARGUMENT );
				break;
		}
		settings->saveScheduleRule(scheduleRule);
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}

	QObject::disconnect( scheduler.get(), SIGNAL( askForPassword( const QString&, bool, int*, const QString& ) ),
						 this, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ) );
	QObject::disconnect( scheduler.get(), SIGNAL( infoSignal( const QString& ) ),
						 this, SLOT( infoDialog( const QString& ) ) );
	QObject::disconnect( scheduler.get(), SIGNAL( errorSignal( const QString& ) ),
						 this, SLOT( errorDialog( const QString& ) ) );
}

QStringList MainModel::getPrefixes()
{
	QStringList prefixes;
	if( !initConnection() )
	{
		return prefixes;
	}
	try
	{
		auto_ptr< AbstractRsync > rsync = ToolFactory::getRsyncImpl();
		prefixes = rsync->getPrefixes();
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return prefixes;
}

QList<int> MainModel::getServerQuota()
{
	qDebug() << "MainModel::getServerQuota()";
	QList<int> quota;
	if( !initConnection() )
	{
		return quota;
	}
	try
	{
		auto_ptr<AbstractRsync> rsync = ToolFactory::getRsyncImpl();
		quota = rsync->getServerQuotaValues();
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return quota;
}

QList<RestoreName> MainModel::getRestoreNames()
{
	QMap<QDate, RestoreName> backupNamesMap;
	QList<RestoreName> backupNames;
	bool wasError = false;
	try
	{
		Settings* settings = Settings::getInstance();
		auto_ptr< AbstractRsync > rsync = ToolFactory::getRsyncImpl();
		QStringList restoreInfoFiles = rsync->downloadAllRestoreInfoFiles( settings->getApplicationDataDir() );
		foreach( QString restoreInfoFile, restoreInfoFiles )
		{
			qDebug() << "restoreInfoFile: (" + restoreInfoFile + ")";
			QFile file( restoreInfoFile );
			if ( file.open( QIODevice::ReadOnly ) )
			{
				UnicodeTextStream in( &file );
				QString dateTimeString = in.readLine();
				QDate date;
				if ( dateTimeString == MainModel::BACKUP_TIME_TODAY )
				{
					date = QDate::currentDate();
				}
				else
				{
					date = QDateTime::fromString( dateTimeString, MainModel::BACKUP_TIME_FORMAT ).date();
				}

				// extract path to backup (e.g. /.snapthots/daily.0 or /incoming )
				QString backupName = restoreInfoFile;
				backupName = backupName.mid( settings->getApplicationDataDir().length(), backupName.lastIndexOf( settings->getBackupPrefix() ) - settings->getApplicationDataDir().length() );

				//backupNamesSet << RestoreName(  backupName, date );
				backupNamesMap.insert(date, RestoreName(  backupName, date ));
			} 
			else 
			{
				wasError = true;
			}
		}
		QMapIterator<QDate, RestoreName> it(backupNamesMap);
		while (it.hasNext())
		{
			it.next();
			backupNames << it.value();
		}
		qSort( backupNames.begin(), backupNames.end() );
		if (wasError)
		{
			emit showCriticalMessageBox( tr( "Some restore info files could not be opened" ) );
		}
		else
		{
			rsync->deleteAllRestoreInfoFiles( settings->getApplicationDataDir() );
		}
		return backupNames;
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return backupNames;
}

QStringList MainModel::getRestoreContent( const QString& backupName )
{
	qDebug() << "MainModel::getRestoreContent( " << backupName << " )";
	QStringList backupContent;
	try
	{
		Settings* settings = Settings::getInstance();
		auto_ptr< AbstractRsync > rsync = ToolFactory::getRsyncImpl();
		QFileInfo backupContentFileName = rsync->downloadBackupContentFile( backupName, settings->getApplicationDataDir() );

		QFile backupContentFile( backupContentFileName.absoluteFilePath() );
		if ( !backupContentFile.open( QIODevice::ReadOnly ) )
		{
			qWarning() << "Can not read backup content file " << backupContentFileName.absoluteFilePath();
			return backupContent; // empty list
		}
		UnicodeTextStream textStream( &backupContentFile );
		while ( !textStream.atEnd() )
		{
			QString line = textStream.readLine();
			FileSystemUtils::convertToLocalPath( &line );
			backupContent << line;
		}
		backupContentFile.remove();
	}
	catch ( ProcessException e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return backupContent;
}

void MainModel::fullRestore( const QString& backupName, const QString& destination )
{
	if( !initConnection() )
	{
		return;
	}
	RestoreThread* restoreThread = new RestoreThread( backupName, destination );

	QObject::connect( restoreThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
						this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( appendInfoMessage( const QString& ) ),
						this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( appendErrorMessage( const QString& ) ),
						this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( finishProgressDialog() ),
						this, SIGNAL( finishProgressDialog() ) );
	QObject::connect( this, SIGNAL( abortProcess() ),
						restoreThread, SLOT( abortRestoreProcess() ) );
	restoreThread->start();
	//TODO: disconnect signal/slot connections
}

void MainModel::customRestore( const QStandardItemModel* remoteDirModel, const QModelIndexList selectionList, const QString& backupName, const QString& destination )
{
	if( !initConnection() )
	{
		return;
	}
	QStringList itemList;
	foreach( QModelIndex selectedItem, selectionList )
	{
		DirTreeItem* item = (DirTreeItem*)remoteDirModel->itemFromIndex( selectedItem );
		itemList << item->getAbsoluteName();
	}
	RestoreThread* restoreThread = new RestoreThread( backupName, itemList, destination );

	QObject::connect( restoreThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
						this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( appendInfoMessage( const QString& ) ),
						this, SIGNAL( appendInfoMessage( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( appendErrorMessage( const QString& ) ),
						this, SIGNAL( appendErrorMessage( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( finishProgressDialog() ),
						this, SIGNAL( finishProgressDialog() ) );
	QObject::connect( this, SIGNAL( abortProcess() ),
						restoreThread, SLOT( abortRestoreProcess() ) );
	restoreThread->start();
	//TODO: disconnect signal/slot connections
}

void MainModel::showCriticalMessage( const QString& message )
{
	emit showCriticalMessageBox( message );
}

void MainModel::showInformationMessage( const QString& message )
{
	emit showInformationMessageBox( message );
}

void MainModel::setStatusBarMessage( const QString& statusBarMessage )
{
	emit updateStatusBarMessage( statusBarMessage );
}

QDirModel* MainModel::getLocalDirModel()
{
	if (!localDirModel)
	{
		localDirModel = new QDirModel( QStringList(),
			QDir::Dirs | QDir::Files | QDir::Drives | QDir::Hidden | QDir::NoDotAndDotDot,
			QDir::Name | QDir::IgnoreCase | QDir::DirsFirst
		);
	}

	return localDirModel;
}

QStandardItemModel* MainModel::getRemoteDirModel( const QString& backupName )
{
	QStringList backupContent = getRestoreContent( backupName );
	return new RemoteDirModel( backupContent );
}

void MainModel::infoSlot( const QString& text )
{
	emit appendInfoMessage( text );
}

void MainModel::errorSlot( const QString& text )
{
	emit appendErrorMessage( text );
}

void MainModel::showProgressDialogSlot( const QString& dialogTitle )
{
	emit showProgressDialog( dialogTitle );
}

void MainModel::closeProgressDialogSlot()
{
	emit closeProgressDialog();
}


void MainModel::infoDialog( const QString& text )
{
	emit showInformationMessageBox( text );
}

void MainModel::errorDialog( const QString& text )
{
	emit showCriticalMessageBox( text );
}

QStringList MainModel::getNewLogfileLines()
{
	return LogFileUtils::getInstance()->getNewLines();
}

void MainModel::exit()
{
	Settings::getInstance()->deletePrivateKeyFiles();
}
