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
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QDir>
#include <QCoreApplication>
#include <QTextCodec>
#include <QTextStream>
#include <QStack>

#include "model/restore_name.hh"
#include "settings/settings.hh"
#include "tools/rsync.hh"
#include "test/test_manager.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/string_utils.hh"

Rsync::Rsync() :
    progress_bytesRead(0),
    progress_bytesWritten(0),
    progress_trafficB_s(0),
    last_calculatedLiteralData(0),
    files_total(-1),
    cur_n_files_done(0),
    lastUpdateTime(QDateTime::currentDateTime())
{
}

Rsync::~Rsync()
{
}

QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > Rsync::upload( const BackupSelectionHash& includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, int bandwidthLimit, QString* warnings, bool dry_run )
{
	qDebug() << "Rsync::upload(" << includeRules << "," << src << "," << destination << "," << setDeleteFlag << "," << compress << "," << (warnings ? *warnings : "(no warnings)") << "," << dry_run << ")";
	enum DryrunStates { DRY_RUN_START, DRY_RUN_COUNT_FILES, DRY_RUN_CALCULATE_SIZE, DRY_RUN_END };
	QRegExp vanishedRegExp("file has vanished: \"([^\"]+)\".*");
	QString STATISTICS_FIRST_USED_LABEL = "Literal data:";
	QString STATISTICS_FIRST_LABEL = "Number of files";
	Settings* settings = Settings::getInstance();
	QString include_dirs_filename = settings->getApplicationDataDir() + "includes";

	QString source(src);
	FileSystemUtils::convertToServerPath( &source );

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncUploadArguments();
	if (Settings::SHOW_PROGRESS) { arguments << getRsyncProgressArguments(); }

	if (dry_run) { 	this->last_calculatedLiteralData = 0; arguments << "-v" << "--stats" << "--only-write-batch=/dev/null"; }

	arguments << getRsyncSshArguments();
	if ( setDeleteFlag )
	{
		arguments << "--del";  // << "--delete-excluded"; // TODO: perhaps --delete-excluded is necessary (temporary inserted [ds] on 2009-08-12, resulted in program-abortion), --del solely doesn't delete files excluded from backup-selection
	}
	if ( compress )
	{
		arguments << "-z";
	}

    if (bandwidthLimit > 0)
        arguments << QString("--bwlimit=%1").arg(bandwidthLimit);

	arguments << source;
	arguments << getValidDestinationPath( destination );
	arguments << "--include-from=-";

	QStringList include_dirs_list;
	QList<QByteArray> convertedRules = calculateRsyncRulesFromIncludeRules(includeRules, &include_dirs_list);
	QList<QByteArray> convertedIncludeDirs;
	foreach ( QString include_dir, include_dirs_list ) { /* qDebug() << "$$$" << include_dir; */ convertedIncludeDirs.append(convertFilenameForRsyncArgument(include_dir)); }
	if (StringUtils::writeQByteArrayListToFile(convertedIncludeDirs, include_dirs_filename, settings->getEOLCharacter())) {
		arguments << "--files-from=" + convertFilenameForRsyncArgument(include_dirs_filename);
		qDebug() << "written directory-names to file" << include_dirs_filename << ":\n" << include_dirs_list;
	} else {
		qDebug() << "unable to write directory-names to file" << include_dirs_filename << ".";
	}

	createProcess( settings->getRsyncName() , arguments );
	start();
	foreach ( QByteArray rule, convertedRules )
	{
		write(rule); // write( convertFilenameForRsyncArgument(rule) );
		qDebug() << rule; // TODO: delete again
		write( settings->getEOLCharacter() );
		waitForBytesWritten();
	}
	closeWriteChannel();

	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > uploadedItems;

	QString lineData;
	bool endReached = false;
	DryrunStates dryrun_state = DRY_RUN_START;
	long nOfFiles = -1;
	QString lastFileName;
	char rsyncEolChar = '\n';
	if (Settings::SHOW_PROGRESS) rsyncEolChar = '\r';
	while( blockingReadLine( &lineData, 2147483647, rsyncEolChar ) ) // -1 does not work on windows
	{
		// qDebug() << "first in while" << readAllStandardError(); // $$$ delete again
		qDebug() << lineData;
		lineData.replace( settings->getEOLCharacter(), "");
		if (lineData.size() > 0) {
			endReached = (endReached || (lineData.contains(STATISTICS_FIRST_LABEL)));
			if (lineData.contains(STATISTICS_FIRST_USED_LABEL)) {
				endReached = true; // hopefully not necessary
				dryrun_state = DRY_RUN_COUNT_FILES;
				this->last_calculatedLiteralData = lineData.mid(STATISTICS_FIRST_USED_LABEL.length()).trimmed().split(" ")[0].toLong();
			}
			QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> item = getItemAndStoreTransferredBytes( lineData );

			if (dry_run && dryrun_state == DRY_RUN_START && lineData.contains("building file list ...")) {
				dryrun_state = DRY_RUN_COUNT_FILES;
			}
			if (dry_run && dryrun_state == DRY_RUN_COUNT_FILES && lineData.contains("files to consider")) {
				dryrun_state = DRY_RUN_CALCULATE_SIZE;
				nOfFiles = lineData.trimmed().split(" ")[0].toLong();
			}
			if (dry_run && dryrun_state == DRY_RUN_COUNT_FILES && lineData.contains(" files...")) {
				int curFileNr = lineData.trimmed().split(" ")[0].toLong();
				this->files_total = curFileNr;
				bufferedInfoOutput(); // prevents direct flush
			}
			if (dry_run && dryrun_state == DRY_RUN_CALCULATE_SIZE) {
				if (!lineData.startsWith("<f") && !lineData.startsWith("cd") ) {
					if (lineData.contains("to-check=")) {
						QStringList files_cur_total = lineData.trimmed().split("to-check=").last().split("/");
						this->files_total = nOfFiles; // files_cur_total.last().replace(")","").toLong();
						this->cur_n_files_done = files_total - files_cur_total.first().toLong();
						this->progress_lastFilename = lastFileName;
					}
				} else {
					lastFileName = lineData.mid(10).trimmed();
				}
				bufferedInfoOutput(); // prevents direct flush
			}

			if (!dry_run) {
				if (!endReached && item.first != "./") {
					item.first.prepend( "/" );
					this->progress_lastFilename = item.first;
					removeSymlinkString( &item.first );
					FileSystemUtils::convertToLocalPath( &item.first );
					if (item.second != UNKNOWN) uploadedItems << item;
					QString outputText;
					switch( item.second )
					{
						case TRANSFERRED:
							outputText = tr( "Uploading %1" ).arg( item.first.trimmed() );
							emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
							break;
						case SKIPPED:
							outputText = tr( "Skipping %1" ).arg( item.first.trimmed() );
							break;
						case DELETED:
							outputText = tr( "Deleting %1" ).arg( item.first.trimmed() );
							break;
						default:
							if (vanishedRegExp.exactMatch(lineData))
							{
								if (warnings)
								{
									warnings->append(lineData);
									warnings->append('\n');
								}
							}
							break;
					}
					qDebug() << outputText; // TODO: comment out again
					emit infoSignal( outputText );
				} else {
					emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
				}
			}
		}
	}
	qDebug() <<  QString::number( uploadedItems.size() ) << "files and/or directories processed";
	emit infoSignal( tr( "%1 files and/or directories processed" ).arg( uploadedItems.size() ) );
	waitForFinished();

	processWarningsAndErrors(warnings);

	return uploadedItems;
}


void Rsync::updateInfoOutput() {
	// qDebug() << "Rsync::updateVolumeOutput()";
	emit volumeCalculationInfoSignal( this->progress_lastFilename, this->files_total, this->cur_n_files_done );
}
void Rsync::bufferedInfoOutput() // prevents direct flush
{
	const int msecs_to_wait_for_flush = 250;

	QDateTime currentTime = QDateTime::currentDateTime();
	if ( this->lastUpdateTime.addMSecs(msecs_to_wait_for_flush) <= currentTime )
	{
		qDebug() << "soft Update success" << msecs_to_wait_for_flush << this->lastUpdateTime;
		// update, but only after half a second, such that following info arriving in the next second are also flushed
		this->lastUpdateTime = currentTime;
		// QTimer::singleShot(msecs_to_wait_for_flush, this, SLOT(updateInfoOutput())); // funktioniert unerklaerlicherweise nicht
		updateInfoOutput();
	}
}

/**
 * deprecated
 */
QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > Rsync::upload( const QStringList& items, const QString& src, const QString& destination, const QStringList& includePatternList, const QStringList& excludePatternList, bool setDeleteFlag, bool compress, int bandwidthLimit, QString* warnings, bool dry_run )
{
	QString STATISTICS_FIRST_USED_LABEL = "Literal data:";
	QString STATISTICS_FIRST_LABEL = "Number of files:";
	qDebug() << "Rsync::upload(" << items << ", " << src << ", " << destination << ", " << includePatternList << ", " << excludePatternList << ")";
	Settings* settings = Settings::getInstance();

	QString source(src);
	FileSystemUtils::convertToServerPath( &source );

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncUploadArguments();
	if (dry_run) { 	this->last_calculatedLiteralData = 1; arguments << "--stats" << "--only-write-batch=/dev/null"; }
	arguments << "--files-from=-";
	arguments << getRsyncSshArguments();
	if ( setDeleteFlag )
	{
		arguments << "--del";
	}
	if ( compress )
	{
		arguments << "-z";
	}

    if (bandwidthLimit > 0)
        arguments << QString("--bwlimit=%1").arg(bandwidthLimit);

	arguments << source;
	arguments << getValidDestinationPath( destination );

	if ( includePatternList.size() > 0 )
	{
		foreach( QString includePattern, includePatternList )
		{
			FileSystemUtils::convertToServerPath( &includePattern );
			arguments << "--include=" + includePattern;
		}
	}

	if ( excludePatternList.size() > 0 )
	{
		foreach( QString excludePattern, excludePatternList )
		{
			FileSystemUtils::convertToServerPath( &excludePattern );
			arguments << "--exclude=" + excludePattern;
		}
	}

	createProcess( settings->getRsyncName() , arguments );
	start();
	foreach ( QString item, items )
	{
		FileSystemUtils::convertToServerPath( &item );

#ifdef Q_OS_MAC
        QString outputString = item.normalized( QString::NormalizationForm_D );
        QByteArray output = outputString.toUtf8();
        write( output );
#elif defined Q_OS_WIN32
        write( item.toUtf8() );
#else
        write( item.toLocal8Bit() );
#endif
		write( settings->getEOLCharacter() );
		waitForBytesWritten();
	}

	closeWriteChannel();

	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > uploadedItems;

	QString lineData;
	bool endReached = false;
	while( blockingReadLine( &lineData, 2147483647 ) ) // -1 does not work on windows
	{
		//qDebug() << lineData;
		lineData.replace( settings->getEOLCharacter(), "");
		endReached = endReached || lineData.contains(STATISTICS_FIRST_LABEL);
		if (lineData.contains(STATISTICS_FIRST_USED_LABEL)) {
			endReached = true; // hopefully not necessary
			this->last_calculatedLiteralData = lineData.mid(STATISTICS_FIRST_USED_LABEL.length()).trimmed().split(" ")[0].toLong();
		}
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> item = getItemAndStoreTransferredBytes( lineData );
		if (!endReached && item.first != "") {
			item.first.prepend( "/" );
			removeSymlinkString( &item.first );
			FileSystemUtils::convertToLocalPath( &item.first );
			this->progress_lastFilename = item.first;
			uploadedItems << item;
			QString outputText;
			switch( item.second )
			{
				case TRANSFERRED:
					outputText = tr( "Uploading %1" ).arg( item.first );
					emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
					break;
				case SKIPPED:
					outputText = tr( "Skipping %1" ).arg( item.first );
					break;
				case DELETED:
					outputText = tr( "Deleting %1" ).arg( item.first );
					break;
				default:
					break;
			}
			// qDebug() << outputText;
			emit infoSignal( outputText );
		} else {
			emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
		}
	}
	qDebug() <<  QString::number( uploadedItems.size() ) << "files and/or directories processed";
	emit infoSignal( tr( "%1 files and/or directories processed" ).arg( uploadedItems.size() ) );
	waitForFinished();

	processWarningsAndErrors(warnings);

	return uploadedItems;
}


long Rsync::calculateUploadTransfer( const BackupSelectionHash includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, int bandwidthLimit, QString* /* errors */, QString* warnings )
{
	this->upload( includeRules, src, destination, setDeleteFlag, compress, bandwidthLimit, warnings, true );
	return this->last_calculatedLiteralData;
}


QStringList Rsync::downloadFullBackup( const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadFullBackup(" << backup_prefix << "," << backupName << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + backup_prefix + "/" + settings->getBackupFolderName() + "/*";
	return download( source, destination, false );
}

QStringList Rsync::downloadCustomBackup( const QString& backup_prefix, const QString& backupName, const QStringList& itemList, const QString& destination )
{
	qDebug() << "Rsync::downloadCustomBackup(" << backupName << ", ..., " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + backup_prefix + "/" + settings->getBackupFolderName() + "/";
	return download( source, destination, itemList, false, true );
}

QStringList Rsync::downloadCustomBackup( const QString& backup_prefix, const QString& backupName, const BackupSelectionHash& selectionRules, const QString& destination )
{
	qDebug() << "Rsync::downloadCustomBackup(" << backupName << ", ..., " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + backup_prefix + "/" + settings->getBackupFolderName() + "/";
	return this->download( source, destination, selectionRules, false, true );
}

QFileInfo Rsync::downloadBackupContentFile( const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadBackupContentFile(" << backup_prefix << "," << backupName << "," << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + backup_prefix + "/" + settings->getMetaFolderName() + "/";

	downloadSingleFile( source, destination, settings->getBackupContentFileName(), true, true );
	return destination + settings->getBackupContentFileName();
}

QFileInfo Rsync::downloadCurrentBackupContentFile( const QString& destination, bool emitErrorSignal )
{
	qDebug() << "Rsync::downloadCurrentBackupContentFile(" << destination << ")";
	Settings* settings = Settings::getInstance();

	QString source = settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getMetaFolderName() + "/";

	downloadSingleFile( source, destination, settings->getBackupContentFileName(), true, emitErrorSignal );
	return destination + settings->getBackupContentFileName();
}

QFileInfo Rsync::downloadMetadata( const QString& backup_prefix, const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadMetadata(" << backup_prefix << "," << backupName << "," << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + backup_prefix + "/" + settings->getMetaFolderName() + "/";

	return downloadSingleFile( source, destination, settings->getMetadataFileName(), true, false );
	// return destination + settings->getMetadataFileName(); // like that, it is not clear, if metadata was successfully downloaded or not
}

QFileInfo Rsync::downloadCurrentMetadata( const QString& destination, bool emitErrorSignal )
{
	qDebug() << "Rsync::downloadCurrentMetadata(" << destination << ")";
	Settings* settings = Settings::getInstance();

	QString source = settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getMetaFolderName() + "/";

	downloadSingleFile( source, destination, settings->getMetadataFileName(), true, emitErrorSignal );
	return destination + settings->getMetadataFileName();
}

QFileInfo Rsync::downloadSingleFile( const QString& source, const QString& destination, const QFileInfo& fileName, bool compress, bool emitErrorSignal )
{
	QStringList downloadedItems = download( source, destination, QStringList( fileName.fileName() ), compress, emitErrorSignal );
	if ( downloadedItems.size() != 1 )
	{
		qWarning() << "Download error for file " << fileName.fileName();
		qDebug() << "Downloaded file(s): " << downloadedItems;
		return QFileInfo(""); // destination + "/" + fileName.fileName(); // now returns an empty QString, if download failed, so that no old metadata gets applied
	}
	if (downloadedItems.at( 0 ) != "") {
		return destination + "/" + downloadedItems.at( 0 );
	} else {
		return QFileInfo("");
	}
}

QStringList Rsync::download( const QString& source, const QString& destination, bool compress )
{
	return download( source, destination, QStringList(), compress, true );
}

/**
 * only used for single files, not for whole restore-process anymore
 */
QStringList Rsync::download( const QString& source, const QString& destination, const QStringList& customItemList, bool compress, bool emitErrorSignal )
{
	qDebug() << "Rsync::download(" << source << ", " << destination << ", ... )";

	Settings* settings = Settings::getInstance();
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(source, "'");

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	arguments << getRsyncSshArguments();
	if (compress) arguments << "-z";
	arguments << QDir::cleanPath( src );
	arguments << getValidDestinationPath( destination );

	if ( customItemList.size() > 0 )
	{
		// read download list from input
		arguments << "--files-from=-";
		createProcess( settings->getRsyncName() , arguments );
		start();
		foreach ( QString item, customItemList )
		{
			FileSystemUtils::convertToServerPath( &item );
#ifdef Q_OS_WIN32
            write( item.toUtf8() );
#elif defined Q_OS_MAC
            QString outputString = item.normalized( QString::NormalizationForm_D );
            QByteArray output = outputString.toUtf8();
            write( output );
#else
            write( item.toLocal8Bit() );
#endif
			write( settings->getEOLCharacter() );
			waitForBytesWritten();
		}
		closeWriteChannel();
	}
	else
	{
		createProcess( settings->getRsyncName() , arguments );
		start();
	}
	QStringList downloadedItems;

	QString lineData;
	while( blockingReadLine( &lineData, 2147483647 ) ) // -1 does not work on windows
	{
		lineData.replace( settings->getEOLCharacter(), "");
		QString item = getItemAndStoreTransferredBytes( lineData ).first;
		if (item != "") {
			removeSymlinkString( &item );
			FileSystemUtils::convertToLocalPath( &item );
			downloadedItems << item;
			QString output = tr( "Downloading %1" ).arg( item );
			emit infoSignal( output );
		} else {
			emit trafficInfoSignal(this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten);
		}
	}
//	qDebug() << "Downloaded Items: " << downloadedItems;
	emit infoSignal( tr( "%1 files and/or directories downloaded" ).arg( downloadedItems.size() ) );
	waitForFinished();

	QString errors = readAllStandardError();
	if ( errors != "" )
	{
		qWarning() << "Error occurred while downloading: " << errors;
		if ( emitErrorSignal )
		{
			throw ProcessException( tr ( "Error occurred while downloading: " ) + errors );
		}
	}
	if (this->exitCode() != 0)
	{
		if ( emitErrorSignal )
		{
			throw ProcessException( QObject::tr( "rsync exited with with exitCode %1 (%2 %3).").arg(this->exitCode() ).arg(errors).arg(readAllStandardOutput().data()) );
		}
	}
	return downloadedItems;
}

QStringList Rsync::download( const QString& source, const QString& destination, const BackupSelectionHash& includeRules, bool compress, bool emitErrorSignal )
{
	qDebug() << "Rsync::download(" << source << ", " << destination << "," << includeRules << ", ... )";

	Settings* settings = Settings::getInstance();
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(source, "'");

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	if (Settings::SHOW_PROGRESS) { arguments << getRsyncProgressArguments(); }
	arguments << getRsyncSshArguments();
	if (compress) arguments << "-z";
	arguments << QDir::cleanPath( src ) + "/";
	arguments << getValidDestinationPath( destination ) + "/";

	if ( includeRules.size() > 0 )
	{
		QString includesFilename = settings->getApplicationDataDir() + "restore_includes";
		if (StringUtils::writeQByteArrayListToFile( calculateRsyncRulesFromIncludeRules(includeRules), includesFilename, QByteArray(settings->getEOLCharacter()) )) {
			arguments << "--include-from=" + convertFilenameForRsyncArgument(includesFilename);
		}
		createProcess( settings->getRsyncName() , arguments );
		start();
	}
	else
	{
		createProcess( settings->getRsyncName() , arguments );
		start();
	}
	QStringList downloadedItems;

	char rsyncEolChar = '\n';
	if (Settings::SHOW_PROGRESS) rsyncEolChar = '\r';
	QString lineData;
	while( blockingReadLine( &lineData, 2147483647, rsyncEolChar ) ) // -1 does not work on windows
	{
		lineData.replace( settings->getEOLCharacter(), "");
		// qDebug() << lineData;
		QString item = getItemAndStoreTransferredBytes( lineData ).first;
		if (item != "") {
			removeSymlinkString( &item );
			FileSystemUtils::convertToLocalPath( &item );
			downloadedItems << item;
			QString output = tr( "Downloading %1" ).arg( item );
			emit infoSignal( output );
		} else {
			emit trafficInfoSignal(this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten);
		}
	}
	qDebug() << "Downloaded Items: " << downloadedItems;
	qDebug() << tr( "%1 files and/or directories downloaded" ).arg( downloadedItems.size() );
	emit infoSignal( tr( "%1 files and/or directories downloaded" ).arg( downloadedItems.size() ) );

	waitForFinished();
	if (this->isAlive()) {
		QString standardErrors = readAllStandardError();
		if ( standardErrors != "" )
		{
			qWarning() << "Error occurred while downloading: " << standardErrors;
			// if (errors) *errors = standardErrors; // needed to return error-messages
			if ( emitErrorSignal )
			{
				throw ProcessException( tr ( "Error occurred while downloading: " ) + standardErrors );
			}
		}
	} else {
		if ( this->exitCode() != 0)
		{
			if ( emitErrorSignal )
			{
				throw ProcessException( QObject::tr( "rsync exited with with exitCode %1 (%2 %3).").arg(this->exitCode() ).arg(readAllStandardError().data()).arg(readAllStandardOutput().data()) );
			}
		}
	}
	return downloadedItems;
}

QStringList Rsync::downloadAllRestoreInfoFiles( const QString& destination, const QString& backup_prefix )
{
	qDebug() << "Rsync::downloadAllRestoreInfoFiles(" << destination << ")";
	Settings* settings = Settings::getInstance();

	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":";

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	arguments << getRsyncSshArguments();

	// include backup info file from backup folder
	QString includePath = settings->getBackupRootFolder();
	arguments << "--include=" + includePath;
	includePath += backup_prefix + "/";
	arguments << "--include=" + includePath;
	includePath += settings->getMetaFolderName() + "/";
	arguments << "--include=" + includePath;
	includePath += settings->getBackupTimeFileName();
	arguments << "--include=" + includePath;

	// include backup info file from restore folder
	includePath = settings->getRestoreRootFolder();
	arguments << "--include=" + includePath;
	includePath += "*/";
	arguments << "--include=" + includePath;
	includePath += backup_prefix + "/";
	arguments << "--include=" + includePath;
	includePath += settings->getMetaFolderName() + "/";
	arguments << "--include=" + includePath;
	includePath += settings->getBackupTimeFileName();
	arguments << "--include=" + includePath;

	arguments << "--exclude=*";
	arguments << src;
	arguments << getValidDestinationPath( destination );

	createProcess( settings->getRsyncName() , arguments );
	start();

	QStringList downloadedRestoreInfoFiles;
	QString lineData;
	while( blockingReadLine( &lineData, 2147483647 ) )
	{
		lineData.replace( settings->getEOLCharacter(), "");
		QString item = getItemAndStoreTransferredBytes( lineData ).first;
		if ( !item.contains( "=>" ) && item.contains( settings->getMetaFolderName() + "/" + settings->getBackupTimeFileName() ) )
		{
			downloadedRestoreInfoFiles << destination + item;
		}
	}
//	qDebug() << "Downloaded Items: " << downloadedRestoreInfoFiles;
	waitForFinished();
	return downloadedRestoreInfoFiles;
}

void Rsync::deleteAllRestoreInfoFiles( const QString& path )
{
	qDebug() << "Rsync::deleteAllRestoreInfoFiles()";
	Settings* settings = Settings::getInstance();
	QString backupRootFolder = settings->getBackupRootFolder();
	// remove first slash if exists
	if ( backupRootFolder.startsWith( "/" ) )
	{
		backupRootFolder = backupRootFolder.remove( 0, 1 );
	}
	QString backupFolder = path + backupRootFolder;
	FileSystemUtils::rmDirRecursive( backupFolder );

	QString restoreRootFolder = settings->getRestoreRootFolder();
	// remove first slash if exists
	if ( restoreRootFolder.startsWith( "/" ) )
	{
		restoreRootFolder = restoreRootFolder.remove( 0, 1 );
	}
	QString restoreFolder = path + settings->getRestoreRootFolder();
	FileSystemUtils::rmDirRecursive( restoreFolder );
}

QStringList Rsync::getPrefixes()
{
	qDebug() << "Rsync::getPrefixes()";
	Settings* settings = Settings::getInstance();


	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	arguments << getRsyncSshArguments();

	// include backupRootFolder/*/
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(settings->getBackupRootFolder(), "'");
	arguments << "--include=/*/";
	arguments << "--exclude=*";
	arguments << src;

	createProcess( settings->getRsyncName() , arguments );
	start();

	QStringList prefixes;
	QString lineData;
	while( blockingReadLine( &lineData, 2147483647 ) )
	{
		lineData.replace( settings->getEOLCharacter(), "");
		qDebug() << "Rsync::getPrefixes():" << lineData;

		QString column;
		QTextStream line( &lineData );
		line >> column; // skip permissions
		line >> column; // skip size
		line >> column; // skip modification date
		line >> column; // skip modification time
		column = line.readLine().trimmed(); // take prefix
		if ( column != "." )
		{
			prefixes << column;
		}
	}
	waitForFinished();
	qDebug() << "Rsync::getPrefixes(): prefixes:" << prefixes;
	return prefixes;
}

QStringList Rsync::getRsyncGeneralArguments()
{
	QStringList result;
	result << "--timeout=" + QString::number(Settings::getInstance()->getRsyncTimeout());
#ifdef Q_OS_WIN32
    result << "-iirtxS8";
#else
    result << "-iilrtxHS8";
    result << "--specials";
#endif
	return result;
}

QStringList Rsync::getRsyncUploadArguments()
{
	QStringList result;
	result << "--no-p";
	result << "--no-g";
	result << "--chmod=ugo=rwX";
	return result;
}

QStringList Rsync::getRsyncProgressArguments()
{
	QStringList result;
	result << "--progress";
	return result;
}


QStringList Rsync::getRsyncDownloadArguments()
{
	QStringList result;
	result << "--no-p";
	result << "--no-g";
	result << "--chmod=u=rwX,g-rwx,o-rwx";
	return result;
}


QStringList Rsync::getRsyncSshArguments()
{
	QStringList arguments;
	Settings* settings = Settings::getInstance();
	arguments << "-e";

	if ( settings->useOpenSshInsteadOfPlinkForRsync() )
	{
		QString argument;
		argument.append( StringUtils::quoteText(settings->getSshName(), "'") );
		argument.append(" -i " + StringUtils::quoteText(settings->createPrivateOpenSshKeyFile(), "'"));
		arguments << argument;
	}
	else
	{
		arguments << StringUtils::quoteText(settings->getPlinkName(), "'") + " -i " + StringUtils::quoteText(settings->createPrivatePuttyKeyFile(), "'");
	}
	return arguments;
}

QString Rsync::getValidDestinationPath( const QString& destination )
{
	QString validDestination = destination;
	if ( validDestination.size() > 1 && validDestination.endsWith( "/" ) )
	{
		validDestination = validDestination.left( validDestination.size() -1 );
	}
	FileSystemUtils::convertToServerPath( &validDestination );
	return validDestination;
	// return convertFilenameForRsyncArgument(validDestination);
}

QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> Rsync::getItem( QString rsyncOutputLine )
{
	static QRegExp itemRegExp("([<>ch.][fdLDS][cstpogz. +?]{7}|\\*deleted) (.*)");

	if (itemRegExp.exactMatch(rsyncOutputLine))
	{
		QString flags = itemRegExp.capturedTexts()[1];
		QString itemName = itemRegExp.capturedTexts()[2];

		ITEMIZE_CHANGE_TYPE type;
		switch (flags.at(0).toLatin1())
		{
			case '<':
			case '>':
			case 'c':
			case 'h':
				type = TRANSFERRED;
				break;
			case '.':
				type = SKIPPED;
				break;
			case '*':
				type = DELETED;
				break;
			default:
				type = UNKNOWN;
				break;
		}
		return qMakePair( itemName, type );
	}

	return qMakePair(QString(), UNKNOWN);
}

QList<QByteArray> Rsync::calculateRsyncRulesFromIncludeRules( const BackupSelectionHash& includeRules, QStringList* files_from_list )
{
	qDebug() << "Rsync::calculateRsyncRulesFromIncludeRules(...)";
	LogFileUtils* log = LogFileUtils::getInstance();

	QList<QByteArray> filters;
	QStack<QPair<QString,bool> > unclosedDirs;
	QString curDir = "/";
	QList<QString> includeRulesList = includeRules.keys();
	qSort(includeRulesList);

	qDebug() << "provided selection rules:";
	log->writeLog("provided selection rules:");
	foreach (QString rule, includeRulesList) {
		qDebug() << (includeRules[rule] ? "+" : "-") + QString(" ") + rule;
		log->writeLog( (includeRules[rule] ? "+" : "-") + QString(" ") + rule );
	}

	QString lastRule = "";
	if (files_from_list != 0) { files_from_list->clear(); files_from_list->append("/"); } // clear list and add "/"
	foreach (QString rule, includeRulesList)
	{
		bool ruleMod = includeRules[rule];
		QString ruleParentDir = StringUtils::parentDir(rule);
		QString ruleDir = StringUtils::dirPart(rule);
		// qDebug() << "rule" << (ruleMod?"+":"-") << rule << "( ruleDir" << ruleDir << "curDir" << curDir << ")" << "isSubDir" << isSubDir << "isParentDir" << isParentDir;

		curDir = StringUtils::equalDirPart(curDir,ruleParentDir);
		while (unclosedDirs.size()>0 && !ruleDir.startsWith(unclosedDirs.top().first)) {
			QPair<QString,bool> dirToClose = unclosedDirs.pop();
			if (dirToClose.second || dirToClose.first !=  lastRule) { // don't exclude dir/** bejond dir/
				filters << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
				// qDebug() << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
			} else {
				// qDebug() << "dropped" << dirToClose.first;
			}
		}
		while (curDir!=ruleParentDir) {
			curDir = ruleDir.left(ruleDir.indexOf("/",curDir.length())+1); // next childDir
			if (curDir==StringUtils::dirPart(lastRule) && !includeRules[lastRule]) { // remove exclude before identical include
				filters.removeLast();
			}
			filters << convertRuleToByteArray( curDir,true );
			if (files_from_list != 0) { files_from_list->append(curDir); }
			// qDebug() << convertRuleToByteArray( curDir,true );
		}
		if (rule.endsWith("/")) { // rule is a dir
			filters << convertRuleToByteArray( ruleDir, ruleMod  );
			unclosedDirs.push( QPair<QString,bool>(ruleDir,ruleMod) );
			if (ruleMod) {
				curDir = ruleDir;
				if (files_from_list != 0) { files_from_list->append(curDir); }
			}
		} else {
			filters << convertRuleToByteArray( rule,ruleMod );
			// qDebug() << convertRuleToByteArray( rule,ruleMod );
		}
		lastRule = rule;
	}
	while (unclosedDirs.size() > 0) {
		QPair<QString,bool> dirToClose = unclosedDirs.pop();
		if (dirToClose.second || dirToClose.first !=  lastRule) { // don't exclude dir/** bejond dir/
			filters << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
		}
	}
	filters << convertRuleToByteArray( "**",false );

	if (files_from_list != 0) { qSort((*files_from_list)); }
	qDebug() << "include rules for rsync:";
	log->writeLog("include rules for rsync:");
	foreach (QByteArray filter, filters) {
		qDebug() << filter;
		log->writeLog(filter);
	}
	return filters;
}

QByteArray Rsync::convertFilenameForRsyncArgument(QString filename) {
	FileSystemUtils::convertToServerPath( &filename );
#ifdef Q_OS_WIN32
    return( filename.toUtf8() );
#elif defined Q_OS_MAC
    return(filename.normalized( QString::NormalizationForm_D ).toUtf8());
#else
    return( filename.toLocal8Bit() );
#endif
}

QByteArray Rsync::convertRuleToByteArray(QString rule, bool modifier)
{
	FileSystemUtils::convertToServerPath( &rule );
	QString rule_modifier = modifier ? "+ " : "- ";
#ifdef Q_OS_MAC
    return (rule_modifier + rule.normalized( QString::NormalizationForm_D )).toUtf8();
#elif defined Q_OS_WIN32
    return (rule_modifier + rule).toUtf8();
#else
    return (rule_modifier + rule).toUtf8();
#endif
}

QByteArray Rsync::convertQStringToQByteArray(QString aStr)
{
#ifdef Q_OS_MAC
    return aStr.normalized( QString::NormalizationForm_D ).toUtf8();
#elif defined Q_OS_WIN32
    return aStr.toUtf8();
#else
    return aStr.toUtf8();
#endif
}


void Rsync::testRsyncRulesConversion()
{
	qDebug() << "Rsync rules conversion test";

	BackupSelectionHash includeRules;
	includeRules.insert("/home/dsydler/projects/xx/D/", true);
	includeRules.insert("/home/dsydler/projects/xx/D/DA/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DA/DAF/DAFB/", true);
	includeRules.insert("/home/dsydler/projects/xx/D/DB/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DC/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DD/DDD/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DD/DDD/DDDF/DDDFD/", true);
	includeRules.insert("/home/dsydler/projects/xx/D/DE/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DF/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DG/", false);
	includeRules.insert("/home/dsydler/projects/xx/D/DH/", false);
	//includeRules.insert("/home/dsydler/projects/xx/bigfile", true);
	Rsync::calculateRsyncRulesFromIncludeRules( includeRules );
}

QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> Rsync::getItemAndStoreTransferredBytes( QString rsyncOutputLine )
{
	if (rsyncOutputLine.startsWith(" ") && rsyncOutputLine.contains("%") && rsyncOutputLine.contains("B/s")) {
		QString trafficStr;
		QStringList parts = rsyncOutputLine.simplified().split(" ");
		if (parts.size() >= 1 && parts[parts.size()-1].trimmed().startsWith("(")) { parts.pop_back(); }
		int nParts = parts.size();
		if ( nParts >= 4 ) {
			this->progress_trafficB_s = StringUtils::trafficStr2BytesPerSecond(parts[nParts-4]);
			this->progress_bytesRead = parts[nParts-2].toLongLong();
			this->progress_bytesWritten = parts[nParts-1].toLongLong();
		}
		return qMakePair( QString(""), SKIPPED );
	} else {
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> myPair = getItem( rsyncOutputLine );
		this->progress_lastFilename = myPair.first;
		return myPair;
	}
}

void Rsync::removeSymlinkString( QString* path )
{
	int symlinkPrefixPosition = path->indexOf( " -> " );
	if ( symlinkPrefixPosition > 0 )
	{
		*path = path->left( symlinkPrefixPosition );
	}
}

void Rsync::abort()
{
	qDebug() << "Rsync::abort()";
	terminate();
}

void Rsync::processWarningsAndErrors(QString* warnings)
{
	if (this->isAlive()) {
		QString standardErrors = readAllStandardError();
		if ( standardErrors != "" )
		{
			qWarning() << "Error occurred while uploading: " + standardErrors;
			throw ProcessException(standardErrors);
		}
	} else {
		int exitCode = this->exitCode();
		if (exitCode == 0) return;
		if (warnings)
		{
			switch (exitCode)
			{
				case 23:  // Partial transfer due to error
					*warnings += StringUtils::fromLocalEnc(readAllStandardError());
					*warnings += "\n";
					*warnings += tr("Warning: At least one file could not be backuped up. See above for details.");
					return;
				case 24:  // Partial transfer due to vanished source files
					*warnings += StringUtils::fromLocalEnc(readAllStandardError());
					*warnings += "\n";
					*warnings += tr("Warning: Some files have been renamed, moved or deleted during backup before they could be uploaded to the server.\n"
								   "Usually this concerns temporary files and this warning can be ignored. See above for details.");
					return;
			}
		} else if ( exitCode == 23 )
		{
		    // If there were no warnings, check if it the exit code is 23, if
		    // yes it means that the disk quota is exceeded and the rsync cannot
		    // execute mkstemp
		    *warnings += StringUtils::fromLocalEnc(readAllStandardError());
		    *warnings += "\n";
		    *warnings += tr("Warning: Server disk quota exceeded. No more backups possible");
		    return;
		}

		throw ProcessException( QObject::tr( "rsync exited with exitCode %1 (%2 %3).").arg(exitCode).arg(readAllStandardError().data()).arg(readAllStandardOutput().data()) );
	}
}

void Rsync::testGetItem()
{
	Rsync rsync;
	QString rsyncOutputLine = ".d          local/restore/home/bsantschi/tmp/";
	QString itemName = rsync.getItem( rsyncOutputLine ).first;
	qDebug() << "itemName: " << itemName;
	if ( itemName == "local/restore/home/bsantschi/tmp/" )
	{
		qDebug() << "Test successful";
	}
	else
	{
		qWarning() << "Test failed";
	}
}

void Rsync::testUpload()
{
	Settings* settings = Settings::getInstance();
	QString backup_prefix = settings->getBackupPrefix();
	QStringList files;
	files << "/tmp2/";
	QString source = "/";
	QString destination = settings->getServerUserName() + "@" + settings->getServerName() + ":" + StringUtils::quoteText(settings->getBackupRootFolder() + backup_prefix + "/" + settings->getBackupFolderName() + "/", "'");
	Rsync rsync;
	QString warnings;
	rsync.upload( files, source, destination, QStringList(), QStringList(), false, false, 0, &warnings, false );
	qDebug() << "warnings: " << warnings;
}

void Rsync::testDownloadCurrentMetadata()
{
	Rsync rsync;
	rsync.downloadCurrentMetadata( Settings::getInstance()->getApplicationDataDir(), false );
}

void Rsync::testDownloadAllRestoreInfoFiles()
{
	Rsync rsync;
	QString backup_prefix = Settings::getInstance()->getBackupPrefix();
	rsync.downloadAllRestoreInfoFiles( Settings::getInstance()->getApplicationDataDir(), backup_prefix );
}

void Rsync::testDeleteAllRestoreInfoFiles()
{
	Rsync rsync;
	rsync.deleteAllRestoreInfoFiles( Settings::getInstance()->getApplicationDataDir() );
}

void Rsync::testDownloadBackupContentFile()
{
	Rsync rsync;
	QString backup_prefix = Settings::getInstance()->getBackupPrefix();
	rsync.downloadBackupContentFile( backup_prefix, Settings::getInstance()->getBackupRootFolder(), Settings::getInstance()->getApplicationDataDir() );
}

void Rsync::testGetPrefixes()
{
	Rsync rsync;
	rsync.getPrefixes();
}



namespace
{
	int dummy1 = TestManager::registerTest( "testUpload", Rsync::testUpload );
	int dummy2 = TestManager::registerTest( "testDownloadCurrentMetadata", Rsync::testDownloadCurrentMetadata );
	int dummy3 = TestManager::registerTest( "testDownloadAllRestoreInfoFiles", Rsync::testDownloadAllRestoreInfoFiles );
	int dummy4 = TestManager::registerTest( "testDeleteAllRestoreInfoFiles", Rsync::testDeleteAllRestoreInfoFiles );
	int dummy5 = TestManager::registerTest( "testDownloadBackupContentFile", Rsync::testDownloadBackupContentFile );
	int dummy6 = TestManager::registerTest( "testGetItem", Rsync::testGetItem );
	int dummy7 = TestManager::registerTest( "testGetPrefixes", Rsync::testGetPrefixes );
	int dummy8 = TestManager::registerTest( "testRsyncRulesConversion", Rsync::testRsyncRulesConversion );
}
