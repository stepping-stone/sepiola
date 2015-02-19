/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2012 stepping stone GmbH
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
#include <memory>

#include "settings/settings.hh"
#include "cli/cli_manager.hh"
#include "exception/login_exception.hh"
#include "model/remote_dir_model.hh"
#include "model/local_dir_model.hh"
#include "model/backup_thread.hh"
#include "model/backup_task.hh"
#include "model/dir_tree_item.hh"
#include "model/main_model.hh"
#include "model/remote_dir_model.hh"
#include "model/local_dir_model.hh"
#include "model/restore_thread.hh"
#include "model/space_usage_model.hh"
#include "tools/abstract_rsync.hh"
#include "tools/abstract_metadata.hh"
#include "tools/abstract_ssh.hh"
#include "tools/abstract_scheduler.hh"
#include "tools/rsync.hh"
#include "tools/posix_acl.hh"
#include "tools/tool_factory.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/unicode_text_stream.hh"
#include "tools/filesystem_snapshot.hh"

const QString MainModel::BACKUP_TIME_FORMAT = "yyyy-MM-dd HH:mm";
const QString MainModel::BACKUP_TIME_TODAY = "TODAY";

MainModel::MainModel() :
    localDirModel(nullptr),
    remoteDirModel(nullptr),
    spaceUsageModel(new SpaceUsageModel(this)),
    isLoginAborted(false),
    fsSnapshot( new FilesystemSnapshot() ),
    backupThread(nullptr),
    startInThisThread(true)
{
}

MainModel::~MainModel()
{
	delete localDirModel;
	delete remoteDirModel;
    delete spaceUsageModel;
}

void MainModel::keepSettings()
{
	Settings* settings = Settings::getInstance();
    std::shared_ptr<AbstractScheduler> scheduler(ToolFactory::getSchedulerImpl());
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
    std::shared_ptr<AbstractScheduler> scheduler(ToolFactory::getSchedulerImpl());
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
        std::shared_ptr<AbstractSsh> ssh(ToolFactory::getSshImpl());
		QObject::connect( ssh.get(), SIGNAL( infoSignal( const QString& ) ),
						  this, SIGNAL( infoSignal( const QString& ) ) );
		QObject::connect( ssh.get(), SIGNAL( errorSignal( const QString& ) ),
						  this, SIGNAL( errorSignal( const QString& ) ) );

		emit infoSignal( tr( "Establishing connection ..." ) );
		emit infoSignal( tr( "Validating server's fingerprint ..." ) );
		if ( !ssh->assertCorrectFingerprint() )
		{
			showInformationMessage( tr( "Server fingerprint validation failed" ) );
			return false;
		}
		else
		{
			emit infoSignal( tr( "Server's fingerprint is correct." ) );
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
				} catch ( const LoginException& e )
				{
					emit infoSignal( tr( "Login failed." ) );
					emit showInformationMessageBox( e.what() );
				}
			}

			emit infoSignal( tr( "Trying to login" ) );
			if ( !ssh->loginWithKey() )
			{
				showInformationMessage( tr( "Can not login to the server. Please review your settings." ) );
			}
			else
			{
				emit infoSignal( tr( "Login succeeded." ) );
				qDebug() << "Second login with key was successful";
				isInitialized = true;
			}
		}
		else
		{
			emit infoSignal( tr( "Login with key was successful" ) );
			isInitialized = true;
		}
		emit infoSignal( tr( "Connection established." ) );
		QObject::disconnect( ssh.get(), SIGNAL( infoSignal( const QString& ) ),
							 this, SIGNAL( infoSignal( const QString& ) ) );
	}
	catch ( const ProcessException& e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return isInitialized;
}

void MainModel::backup( const BackupSelectionHash& includeRules, const bool& startInCurrentThread )
{
	qDebug() << "MainModel::backup( " << includeRules << ", " << startInCurrentThread << " )";
	if ( !initConnection() )
	{
		closeProgressDialogSlot();
		return;
	}

	this->startInThisThread = startInCurrentThread;

	// Set the include rules for the filesystem snapshot object
	this->fsSnapshot->setIncludeRules( includeRules );

	// Create a new Backup Thread
	this->backupThread = new BackupThread( includeRules, this->fsSnapshot );

	// Connect the FilesystemSnapshots sendSnapshotDone signal to the local
	// uploadFiles slot so as soon as the snapshot is finished, the files can
	// be uploaded
	QObject::connect( this->fsSnapshot, SIGNAL( sendSnapshotDone(int) ),
	                   this, SLOT( uploadFiles( int ) ) );

	// Connect the info and error signals to the info and error slot
    QObject::connect( this->fsSnapshot, SIGNAL( infoSignal(const QString&) ),
                       this, SLOT( infoSlot(const QString&) ) );

	// Start the fs snapshot
	this->fsSnapshot->doSnapshot();

}

bool MainModel::isSchedulingOnStartSupported()
{
    std::shared_ptr<AbstractScheduler> scheduler(ToolFactory::getSchedulerImpl());
	return scheduler->isSchedulingOnStartSupported();
}

void MainModel::schedule( const BackupSelectionHash& includeRules, const ScheduledTask& scheduleRule )
{
	showProgressDialogSlot( tr( "Verify connection" ) );
	if ( initConnection() )
	{
        try
        {
            // Update the schedule information to the server
            BackupThread backupThread( includeRules );
            backupThread.prepareServerDirectories();
            backupThread.uploadSchedulerXML( scheduleRule );
        }
        catch ( const ProcessException& e )
        {
            emit showCriticalMessageBox( e.what() );
        }

		closeProgressDialogSlot();
	}
	else
	{
		closeProgressDialogSlot();
		emit showCriticalMessageBox( tr( "Task has not been scheduled, because the connection could not be established" ) );
		return;
	}
	Settings* settings = Settings::getInstance();
	settings->saveBackupSelectionRules( includeRules );

    std::shared_ptr<AbstractScheduler> scheduler(ToolFactory::getSchedulerImpl());

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
	catch ( const ProcessException& e )
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
        std::shared_ptr<AbstractRsync> rsync(ToolFactory::getRsyncImpl());
		prefixes = rsync->getPrefixes();
	}
	catch ( const ProcessException& e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return prefixes;
}

const SpaceUsageModel* MainModel::getSpaceUsageModel()
{
    return spaceUsageModel;
}

QList<int> MainModel::getServerQuota()
{
	qDebug() << "MainModel::getServerQuota()";
	QList<int> quota;
	if( !initConnection() )
	{
		return quota;
	}
	//try
	//{
        std::shared_ptr<AbstractSsh> ssh(ToolFactory::getSshImpl());
		quota = ssh->getServerQuotaValues();
	//}
	//catch ( const ProcessException& e )
	//{
	//	emit showCriticalMessageBox( e.what() );
	//}
	return quota;
}

QList<RestoreName> MainModel::getRestoreNames( const QString & backup_prefix )
{
	qDebug() << "MainModel::getRestoreNames(" << backup_prefix << ")";
	QMap<QDate, RestoreName> backupNamesMap;
	QList<RestoreName> backupNames;
	bool wasError = false;
	try
	{
		Settings* settings = Settings::getInstance();
        std::shared_ptr<AbstractRsync> rsync(ToolFactory::getRsyncImpl());
		QStringList restoreInfoFiles = rsync->downloadAllRestoreInfoFiles( settings->getApplicationDataDir(), backup_prefix );
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
				backupName = backupName.mid( settings->getApplicationDataDir().length(), backupName.lastIndexOf( backup_prefix ) - 1 - settings->getApplicationDataDir().length() );

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
	catch ( const ProcessException& e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return backupNames;
}

QStringList MainModel::getRestoreContent( const QString & backup_prefix, const QString& backupName )
{
	qDebug() << "MainModel::getRestoreContent( " << backup_prefix << "," << backupName << " )";
	QStringList backupContent;
	try
	{
		Settings* settings = Settings::getInstance();
        std::shared_ptr<AbstractRsync> rsync(ToolFactory::getRsyncImpl());
		QFileInfo backupContentFileName = rsync->downloadBackupContentFile( backup_prefix, backupName, settings->getApplicationDataDir() );

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
	catch ( const ProcessException& e )
	{
		emit showCriticalMessageBox( e.what() );
	}
	return backupContent;
}

void MainModel::fullRestore( const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	if( !initConnection() )
	{
		return;
	}
	BackupSelectionHash selectionRules;
	selectionRules.insert("/", true);
	RestoreThread* restoreThread = new RestoreThread( backup_prefix, backupName, selectionRules, destination );

	QObject::connect( restoreThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
						this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( infoSignal( const QString& ) ),
					  this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( errorSignal( const QString& ) ),
					  this, SIGNAL( errorSignal( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ),
					  this, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ) );
	QObject::connect( restoreThread, SIGNAL( finishProgressDialog() ),
						this, SIGNAL( finishProgressDialog() ) );
	QObject::connect( this, SIGNAL( abortProcess() ),
						restoreThread, SLOT( abortRestoreProcess() ) );
	restoreThread->start();
	//TODO: disconnect signal/slot connections
}

void MainModel::customRestore( const BackupSelectionHash& selectionRules, const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	if( !initConnection() )
	{
		return;
	}

	RestoreThread* restoreThread = new RestoreThread( backup_prefix, backupName, selectionRules, destination );

	QObject::connect( restoreThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
					  this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( infoSignal( const QString& ) ),
					  this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( errorSignal( const QString& ) ),
					  this, SIGNAL( errorSignal( const QString& ) ) );
	QObject::connect( restoreThread, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ),
					  this, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ) );
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

LocalDirModel* MainModel::getLocalDirModel()
{
	if (!localDirModel)
	{
		if (Settings::getInstance()->getShowHiddenFilesAndFolders()) {
			localDirModel = new LocalDirModel( QStringList(),
					QDir::Dirs | QDir::Files | QDir::Drives | QDir::Hidden | QDir::NoDotAndDotDot,
					QDir::Name | QDir::IgnoreCase | QDir::DirsFirst
			);
		} else {
			localDirModel = new LocalDirModel( QStringList(),
					QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot,
					QDir::Name | QDir::IgnoreCase | QDir::DirsFirst
			);
		}
	}
	return localDirModel;
}

QStandardItemModel* MainModel::getCurrentRemoteDirModel_general()
{
	return getCurrentRemoteDirModel();
}

RemoteDirModel* MainModel::getCurrentRemoteDirModel()
{
	return remoteDirModel;
}

RemoteDirModel* MainModel::loadRemoteDirModel( const QString& backup_prefix, const QString& backupName )
{
	// @TODO there is at the moment no possibility to firce reload
	this->clearRemoteDirModel();
	qDebug() << "MainModel::loadRemoteDirModel   remoteDirModel" << remoteDirModel;
	QStringList backupContent = getRestoreContent( backup_prefix, backupName );
	this->remoteDirModel = new RemoteDirModel( backupContent );
	return this->remoteDirModel;
}

void MainModel::clearRemoteDirModel()
{
	delete this->remoteDirModel;
	this->remoteDirModel = 0;
}

void MainModel::infoSlot( const QString& text )
{
	emit infoSignal( text );
}

void MainModel::errorSlot( const QString& text )
{
	emit errorSignal( text );
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

void MainModel::uploadFiles( int result )
{
    // If the creation of the filesystem snapshot was not successful, clean it
    // up and stop the backup here
    if ( result != SNAPSHOT_SUCCESS )
    {
        this->fsSnapshot->cleanup();
        delete this->fsSnapshot;
        this->backupThread->setLastBackupState( ConstUtils::STATUS_ERROR );
        emit infoSignal( "==================================================" );
        emit finalStatusSignal( ConstUtils::STATUS_ERROR );
        emit finishProgressDialog();
        emit updateOverviewFormLastBackupsInfo();
        emit backupFinished();
        return;
    }

    QObject::connect( backupThread, SIGNAL( backupFinished() ),
                      this, SIGNAL( backupFinished() ) );
    QObject::connect( backupThread, SIGNAL( showCriticalMessageBox( const QString& ) ),
                      this, SIGNAL( showCriticalMessageBox( const QString& ) ) );
    QObject::connect( backupThread, SIGNAL( infoSignal( const QString& ) ),
                      this, SIGNAL( infoSignal( const QString& ) ) );
    QObject::connect( backupThread, SIGNAL( errorSignal( const QString& ) ),
                      this, SIGNAL( errorSignal( const QString& ) ) );
    qRegisterMetaType<StringPairList>("StringPairList");
    qRegisterMetaType<ConstUtils::StatusEnum>("ConstUtils::StatusEnum");
    QObject::connect( backupThread, SIGNAL( progressSignal( const QString&, float, const QDateTime&, StringPairList ) ),
                      this, SIGNAL( progressSignal( const QString&, float, const QDateTime&, StringPairList ) ) );
    QObject::connect( backupThread, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ),
                      this, SIGNAL( finalStatusSignal( ConstUtils::StatusEnum ) ) );
    QObject::connect( backupThread, SIGNAL( finishProgressDialog() ),
                        this, SIGNAL( finishProgressDialog() ) );
    QObject::connect( backupThread, SIGNAL( updateOverviewFormLastBackupsInfo() ),
                      this, SIGNAL ( updateOverviewFormLastBackupsInfo() ) );
    QObject::connect( this, SIGNAL( abortProcess() ),
                        backupThread, SLOT( abortBackupProcess() ) );
    qDebug() << "MainModel::uploadFiles: startInCurrentThread=" << this->startInThisThread;
    if ( this->startInThisThread )
    {
        // signals are not connected with QCoreApplication and multiple threads
        // but in CLI mode we do not need an own thread
        backupThread->startInCurrentThread();
    }
    else
    {
        backupThread->start();
        //TODO: disconnect signal/slot connections
    }
}
