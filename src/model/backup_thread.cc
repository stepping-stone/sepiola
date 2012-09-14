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

#include <QDir>
#include <QPair>
#include <QSet>
#include <QDateTime>

#include "exception/process_exception.hh"
#include "exception/abort_exception.hh"
#include "model/backup_thread.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "tools/tool_factory.hh"
#include "tools/abstract_metadata.hh"
#include "tools/abstract_rsync.hh"
#include "tools/rsync.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/unicode_text_stream.hh"
#include "utils/datetime_utils.hh"
#include "utils/progress_task.hh"

const QString BackupThread::TASKNAME_PREPARE_DIRECTORIES = "preparing server directories";
const QString BackupThread::TASKNAME_ESTIMATE_BACKUP_SIZE = "estimating backup size";
const QString BackupThread::TASKNAME_FILE_UPLOAD = "file upload";
const QString BackupThread::TASKNAME_DOWNLOAD_CURRENT_BACKUP_CONTENT = "downloading current backup content file";
const QString BackupThread::TASKNAME_UPLOAD_METADATA = "uploading metadata";
const QString BackupThread::TASKNAME_METAINFO = "saving meta information";


BackupThread::BackupThread( const BackupSelectionHash& incRules ) :
	rsync(ToolFactory::getRsyncImpl()),
    isAborted(false),
    includeRules(incRules),
    backupStartDateTime(QDateTime()), // null-Time
    backupCurrentStatus(ConstUtils::STATUS_UNDEFINED)
{
	Settings* settings = Settings::getInstance();
	this->setDeleteFlag = settings->getDeleteExtraneousItems();
	this->compressedUpload = settings->isCompressedRsyncTraffic();

	// TODO adjust the given values for subtask steps and durations
	this->pt = ProgressTask("Backup", DateTimeUtils::getDateTimeFromSecs(60), 50);
	pt.appendTask(TASKNAME_PREPARE_DIRECTORIES, DateTimeUtils::getDateTimeFromSecs(2 /* time for directory-preparation */), 3.0);
	pt.appendTask(TASKNAME_ESTIMATE_BACKUP_SIZE, DateTimeUtils::getDateTimeFromSecs(30 /* time for dry run */), 1.0 /* steps??? */);
	pt.appendTask(TASKNAME_FILE_UPLOAD, DateTimeUtils::getDateTimeFromSecs(100), 250/* enter bytes to upload */);
	pt.appendTask(TASKNAME_DOWNLOAD_CURRENT_BACKUP_CONTENT, DateTimeUtils::getDateTimeFromSecs(2 /* backup content download */), 3);
	pt.appendTask(TASKNAME_UPLOAD_METADATA, DateTimeUtils::getDateTimeFromSecs(2), 1.0 /* steps??? */);
	pt.appendTask(TASKNAME_METAINFO, DateTimeUtils::getDateTimeFromSecs(2), 1);
	currentTaskNr = 0;


	QObject::connect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
					  this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::connect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
					  this, SIGNAL( errorSignal( const QString& ) ) );
	QObject::connect( rsync.get(), SIGNAL( trafficInfoSignal( const QString&, float, quint64, quint64 ) ),
					  this, SLOT( rsyncUploadProgressHandler( const QString&, float, quint64, quint64 ) ) );
	QObject::connect( this, SIGNAL( abort_rsync() ), rsync.get(), SLOT( abort() ) );
	QObject::connect( this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

BackupThread::~BackupThread()
{
	qDebug() << "BackupThread::~BackupThread()";
	QObject::disconnect( rsync.get(), SIGNAL( infoSignal( const QString& ) ),
						 this, SIGNAL( infoSignal( const QString& ) ) );
	QObject::disconnect( rsync.get(), SIGNAL( errorSignal( const QString& ) ),
						 this, SIGNAL( errorSignal( const QString& ) ) );
	QObject::disconnect( rsync.get(), SIGNAL( trafficInfoSignal( const QString&, float, quint64, quint64 ) ),
						 this, SLOT( rsyncUploadProgressHandler( const QString&, float, quint64, quint64 ) ) );
	QObject::disconnect( this, SIGNAL( abort_rsync() ),
							 rsync.get(), SLOT( abort() ) );
}

void BackupThread::startInCurrentThread()
{
	run();
}

void BackupThread::run()
{
	qDebug() << "BackupThread::run()";
	ProgressTask* subPt;
	bool failed = false;
	QString warnings;
	this->setLastBackupState(ConstUtils::STATUS_UNDEFINED);
	Settings* settings = Settings::getInstance();
	settings->saveBackupSelectionRules( this->includeRules );
	try {
		checkAbortState();
		emit infoSignal( tr( "Creating/validating server directories" ) );

		prepareServerDirectories();
		checkAbortState();

		QString source = "/";
		QString destination = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/", "'");
		this->pt.debugIsCorrectCurrentTask(TASKNAME_ESTIMATE_BACKUP_SIZE);
		emit infoSignal( QObject::tr( QString(TASKNAME_ESTIMATE_BACKUP_SIZE).toLocal8Bit() ) );
		quint64 backupSize = 1;
		if (Settings::SHOW_PROGRESS) {
			backupSize = this->estimateBackupSize( source, destination );
			qDebug() << "calculated literal data:" << backupSize;
		}
		if ((subPt = this->pt.getSubtask(TASKNAME_ESTIMATE_BACKUP_SIZE)) != 0) subPt->setTerminated(true);


		this->pt.debugIsCorrectCurrentTask(TASKNAME_FILE_UPLOAD);
		emit infoSignal( tr( "Uploading files and directories" ) );
		QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems = rsync->upload( includeRules, source, destination, setDeleteFlag, compressedUpload, &warnings, false );

		checkAbortState();
		if ((subPt = this->pt.getSubtask(TASKNAME_FILE_UPLOAD)) != 0) subPt->setTerminated(true);

		if ( processedItems.size() > 0 )
		{
			QString metaDataDir = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getMetaFolderName(), "'");

			// backup content file list
			this->pt.debugIsCorrectCurrentTask(TASKNAME_DOWNLOAD_CURRENT_BACKUP_CONTENT);
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getBackupContentFileName() );

			emit infoSignal( tr( "Getting backup content meta data" ) );
			QFileInfo currentBackupContentFile = rsync->downloadCurrentBackupContentFile( settings->getApplicationDataDir(), false );
			checkAbortState();
			if ((subPt = this->pt.getSubtask(TASKNAME_DOWNLOAD_CURRENT_BACKUP_CONTENT)) != 0) subPt->setTerminated(true);

			emit infoSignal( tr( "Uploading backup content meta data" ) );
			this->pt.debugIsCorrectCurrentTask(TASKNAME_UPLOAD_METADATA);
			updateBackupContentFile( currentBackupContentFile, processedItems );

			QString warning;
			rsync->upload( currentBackupContentFile, metaDataDir, true, 0 );

			checkAbortState();
			if ((subPt = this->pt.getSubtask(TASKNAME_UPLOAD_METADATA)) != 0) subPt->setTerminated(true);

			// backup time file
			emit infoSignal( tr( "Uploading backup info data" ) );
			this->pt.debugIsCorrectCurrentTask(TASKNAME_METAINFO);
			QString currentBackupTimeFile = createCurrentBackupTimeFile();

			rsync->upload( currentBackupTimeFile, metaDataDir, true, 0 );
			FileSystemUtils::removeFile( currentBackupTimeFile );
			checkAbortState();

			// permission file
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getMetadataFileName() );
			FileSystemUtils::removeFile( settings->getApplicationDataDir() + settings->getTempMetadataFileName() );
			emit infoSignal( tr( "Getting permission meta data" ) );
			auto_ptr< AbstractMetadata > metadata = ToolFactory::getMetadataImpl();
			QObject::connect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
							  this, SIGNAL( infoSignal( const QString& ) ) );
			QObject::connect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
							  this, SIGNAL( errorSignal( const QString& ) ) );
			QFileInfo currentMetadataFileName = rsync->downloadCurrentMetadata( settings->getApplicationDataDir(), false );
			checkAbortState();
			QFileInfo newMetadataFileName = metadata->getMetadata( settings->getApplicationDataDir() + settings->getTempMetadataFileName(), processedItems, &warning );
			if (!warning.isEmpty()) warnings.append(warning);
			checkAbortState();
			metadata->mergeMetadata( newMetadataFileName, currentMetadataFileName, processedItems );
			checkAbortState();
			emit infoSignal( tr( "Uploading permission meta data" ) );

			try
			{
				rsync->upload( currentMetadataFileName, metaDataDir, true, &warning );
			}
			catch ( const ProcessException& e )
			{
				warning = e.what();
			}
			if (!warning.isEmpty())
			{
				warnings.append("\n");
				warnings.append(tr("Warning: Could not backup file permissions due to following problem:"));
				warnings.append("\n");
				warnings.append(warning);
			}
			checkAbortState();
			if ((subPt = this->pt.getSubtask(TASKNAME_METAINFO)) != 0) { subPt->addFixpointNow(subPt->getNumberOfSteps()); subPt->setTerminated(true); }
			updateInformationToDisplay();

			QObject::disconnect( metadata.get(), SIGNAL( infoSignal( const QString& ) ),
								 this, SIGNAL( infoSignal( const QString& ) ) );
			QObject::disconnect( metadata.get(), SIGNAL( errorSignal( const QString& ) ),
								 this, SIGNAL( errorSignal( const QString& ) ) );
			if( !failed && !isAborted)
			{
				if (warnings.isEmpty())
				{
					emit infoSignal( tr( "Backup succeeded." ) );
				}
				else
				{
					emit infoSignal(" ");
					emit infoSignal(warnings);
					emit infoSignal(" ");
					emit infoSignal( tr( "Backup succeeded with warnings." ) );
				}
			}
		}
	}
	catch ( const ProcessException& e )
	{
		failed = true;
		emit errorSignal( e.what() );
	}
	catch ( const AbortException& e )
	{
		qDebug() << "Backup aborted.";
		emit infoSignal( tr( "Backup aborted." ) );
		qDebug() << e.what();
		emit errorSignal( e.what() );
	}
	if ( failed )
	{
		emit infoSignal( tr( "Backup failed." ) );
		this->setLastBackupState(ConstUtils::STATUS_ERROR);
		qDebug() << "BackupThread::run()" << "finalStatusSignal( ConstUtils::STATUS_ERROR )";

	}
	else if (!warnings.isEmpty())
	{
		this->setLastBackupState(ConstUtils::STATUS_WARNING);
	} else {
		this->setLastBackupState(ConstUtils::STATUS_OK);
		qDebug() << "BackupThread::run()" << "finalStatusSignal( ConstUtils::STATUS_OK )";
	}
	emit infoSignal( "==================================================" );
	emit finalStatusSignal( this->getLastBackupState() );
	emit finishProgressDialog();
	emit updateOverviewFormLastBackupsInfo();
}

void BackupThread::checkAbortState()
{
	if ( isRunning() && isAborted )
	{
		this->setLastBackupState(ConstUtils::STATUS_ERROR);
		qDebug() << "BackupThread::checkAbortState(): Backup has been aborted";
		throw AbortException( tr( "Backup has been aborted" ) );
	}
}

void BackupThread::setLastBackupState(ConstUtils::StatusEnum newStatus)
{
	qDebug() << "BackupThread::setLastBackupState(" << (int)newStatus << ")";
	Settings* settings = Settings::getInstance();
	if (this->backupStartDateTime.isNull()) // new Backup
	{
		qDebug() << "overwriting last backup state";
		this->backupStartDateTime = QDateTime::currentDateTime();
		this->backupCurrentStatus = newStatus;
	} else {
		qDebug() << "creating new last backup state object";
		// backupState cannot become "better", e.g. go from WARNINGS to OK during one backup
		this->backupCurrentStatus = (ConstUtils::StatusEnum)std::max( (int)newStatus, (int)(this->backupCurrentStatus) );
	}
	QList<BackupTask> savedBkupInfos = settings->getLastBackups();
	if (!savedBkupInfos.empty() && savedBkupInfos.first().getDateTime() == this->backupStartDateTime) {
		this->backupStartDateTime = QDateTime::currentDateTime();
		settings->replaceLastBackup( BackupTask(this->backupStartDateTime, newStatus) );
	} else {
		this->backupStartDateTime = QDateTime::currentDateTime();
		settings->addLastBackup( BackupTask(this->backupStartDateTime, newStatus) );
	}
}

ConstUtils::StatusEnum BackupThread::getLastBackupState()
{
	return this->backupCurrentStatus;
}

void BackupThread::rsyncDryrunProgressHandler(const QString& filename, long files_total, long files_done) {
	ProgressTask* mySubPt = this->pt.getSubtask(TASKNAME_ESTIMATE_BACKUP_SIZE); // this->pt.getCurrentTask();
	StringPairList vars = StringPairList();
	QString currentTaskName = mySubPt!=0 ? mySubPt->getName() : "";
	if (currentTaskName==TASKNAME_ESTIMATE_BACKUP_SIZE)
	{
		double ratio_done = (double)files_done / (files_total!=0 ? files_total: 1);
		mySubPt->addFixpointNow(ratio_done);
		vars.append( QPair<QString,QString>(tr("current task"), mySubPt->getName()));
		vars.append( QPair<QString,QString>(tr("current file"), StringUtils::filenameShrink(QDir::toNativeSeparators(filename),72)) );
		vars.append( QPair<QString,QString>(tr("files processed"), QString("%1 / %2").arg(QString::number(files_done),QString::number(files_total))) );
		vars.append( QPair<QString,QString>(tr("estimated remaining time")+":", mySubPt->getRootTask()->getEstimatedTimeLeftString()) );
	}
	updateInformationToDisplay(vars);
}

void BackupThread::rsyncUploadProgressHandler(const QString& filename, float traffic, quint64 bytesRead, quint64 bytesWritten) {
	ProgressTask* mySubPt = this->pt.getCurrentTask();
	StringPairList vars = StringPairList();
	QString currentTaskName = mySubPt->getName();
	if (currentTaskName==TASKNAME_FILE_UPLOAD || currentTaskName==TASKNAME_UPLOAD_METADATA || currentTaskName==TASKNAME_ESTIMATE_BACKUP_SIZE)
	{
		mySubPt->addFixpointNow(bytesWritten);
		vars.append( QPair<QString,QString>(tr("current task"), mySubPt->getName()));
		vars.append( QPair<QString,QString>(tr("current file"), filename) );
		vars.append( QPair<QString,QString>(tr("traffic / transferred")+":", StringUtils::bytesToReadableStr((double)traffic,"B/s") + " / " + StringUtils::bytesToReadableStr((double)bytesWritten+bytesRead, "B")) );
		vars.append( QPair<QString,QString>(tr("estimated remaining time")+":", mySubPt->getRootTask()->getEstimatedTimeLeftString()) );
	}
	updateInformationToDisplay(vars);
}

void BackupThread::updateInformationToDisplay(StringPairList vars)
{
	ProgressTask* mySubPt = this->pt.getCurrentTask();
	emit progressSignal(QObject::tr(mySubPt->getName().toUtf8().data()), mySubPt->getRootTask()->getFinishedRatio(), mySubPt->getRootTask()->getEstimatedTimeLeft(), vars);
}


void BackupThread::prepareServerDirectories()
{
	const QString infoText = QObject::tr("creating server directories");
	ProgressTask * mySubPt = this->pt.getCurrentTask();
	this->pt.debugIsCorrectCurrentTask(TASKNAME_PREPARE_DIRECTORIES);
	mySubPt->setNumberOfSteps(3);
	mySubPt->addFixpointNow(0);
	// create local directories and upload them
	Settings* settings = Settings::getInstance();
	QString source = settings->getApplicationDataDir();
	QString destination = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(settings->getBackupRootFolder(), "'"); // $$$ quotes necessary?
	QDir workingDir( settings->getApplicationDataDir() );
	QString backupPrefixFolder = settings->getBackupPrefix();
	workingDir.mkdir( backupPrefixFolder );
	QDir backupPrefixDir( settings->getApplicationDataDir() + settings->getBackupPrefix() );
	checkAbortState();
	mySubPt->addFixpointNow(1);

	StringPairList vars;
	vars.append(QPair<QString,QString>(tr("current task"), infoText));
	emit progressSignal(mySubPt->getName(), mySubPt->getRootTask()->getFinishedRatio(), mySubPt->getRootTask()->getEstimatedTimeLeft(), vars);

	rsync->upload( QStringList( backupPrefixFolder ), source, destination, QStringList(), QStringList(), false, false, 0, false );
	checkAbortState();
	backupPrefixDir.mkdir( settings->getMetaFolderName() );
	backupPrefixDir.mkdir( settings->getBackupFolderName() );
	rsync->upload( QStringList( backupPrefixFolder ), source, destination, QStringList(), QStringList(), false, false, 0, false );
	checkAbortState();
	mySubPt->addFixpointNow(2);
	emit progressSignal(mySubPt->getName(), mySubPt->getRootTask()->getFinishedRatio(), mySubPt->getRootTask()->getEstimatedTimeLeft(), vars);

	// delete local directories
	backupPrefixDir.rmdir( settings->getMetaFolderName() );
	backupPrefixDir.rmdir( settings->getBackupFolderName() );
	workingDir.rmdir( backupPrefixFolder );
	mySubPt->addFixpointNow(3);
	emit progressSignal(mySubPt->getName(), mySubPt->getRootTask()->getFinishedRatio(), mySubPt->getRootTask()->getEstimatedTimeLeft(), vars);
	mySubPt->setTerminated(true);
}

/**
 * estimates the backup size by running rsync with option --only-write-batch ans updates the progressTask's stepNumber to this size.
 */
quint64 BackupThread::estimateBackupSize( const QString& src, const QString& destination ) {
	ProgressTask * mySubPt = this->pt.getCurrentTask();
	StringPairList vars;
	vars.append(QPair<QString,QString>(tr("current task"), mySubPt->getName()));
	emit progressSignal(mySubPt->getName(), mySubPt->getRootTask()->getFinishedRatio(), mySubPt->getRootTask()->getEstimatedTimeLeft(), vars);

	QObject::connect( rsync.get(), SIGNAL( volumeCalculationInfoSignal( const QString&, long, long ) ),
					  this, SLOT( rsyncDryrunProgressHandler( const QString&, long, long ) ) );
	quint64 uploadSize = rsync->calculateUploadTransfer( includeRules, src, destination, false, false, NULL, NULL );
	QObject::disconnect( rsync.get(), SIGNAL( volumeCalculationInfoSignal( const QString&, long, long ) ),
					  this, SLOT( rsyncDryrunProgressHandler( const QString&, long, long ) ) );

	ProgressTask* uploadPt = this->pt.getSubtask(TASKNAME_FILE_UPLOAD);
	if (uploadPt != 0) uploadPt->setNumberOfSteps(uploadSize);

	return uploadSize;
}

void BackupThread::updateBackupContentFile( const QFileInfo& backupContentFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& backupList )
{
	qDebug() << "BackupThread::updateBackupContentFile(" << backupContentFileName.absoluteFilePath() << ", [backupList] )";
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
		// qDebug() << item;
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
	this->setLastBackupState(ConstUtils::STATUS_ERROR);
	emit abort_rsync();
	emit updateOverviewFormLastBackupsInfo();
	emit finalStatusSignal( this->getLastBackupState() );
}
