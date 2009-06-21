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
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QString>
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

Rsync::Rsync() {}

Rsync::~Rsync() {}

/**
 * deprecated
 */
QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > Rsync::upload( const QHash<QString,bool>& includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, QString* errors, bool dry_run ) throw ( ProcessException )
{
	QString STATISTICS_FIRST_USED_LABEL = "Literal data:";
	QString STATISTICS_FIRST_LABEL = "Number of files";
	qDebug() << "Rsync::upload(" << includeRules << ", " << src << ", " << destination << ", " << setDeleteFlag << ", " << compress << ", " << errors << ", " << dry_run << ")";
	Settings* settings = Settings::getInstance();

	QString source(src);
	FileSystemUtils::convertToServerPath( &source );

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncUploadArguments();

	if (dry_run) { 	this->last_calculatedLiteralData = -1; arguments << "--stats" << "--only-write-batch=/dev/null"; }
	
	arguments << "--include-from=-";
	arguments << getRsyncSshArguments();
	if ( setDeleteFlag )
	{
		arguments << "--del";
	}
	if ( compress )
	{
		arguments << "-z";
	}
	arguments << source;
	arguments << getValidDestinationPath( destination );

	createProcess( settings->getRsyncName() , arguments );
	start();
	
	QList<QByteArray> convertedRules = calculateRsyncRulesFromIncludeRules(includeRules);
	foreach ( QByteArray rule, convertedRules )
	{
		write( rule );
		write( settings->getEOLCharacter() );
		waitForBytesWritten();
	}
	closeWriteChannel();

	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > uploadedItems;

	QString lineData;
	bool endReached = false;
	while( blockingReadLine( &lineData, 2147483647, 13 ) ) // -1 does not work on windows
	{
		//qDebug() << lineData;
		lineData.replace( settings->getEOLCharacter(), "");
		endReached = (endReached || (lineData.contains(STATISTICS_FIRST_LABEL)));
		// /*$$$DEBUG*/if (endReached) { qDebug() << "Rsync::upload(...)" << lineData; }
		if (lineData.contains(STATISTICS_FIRST_USED_LABEL)) {
			endReached = true; // hopefully not necessary
			this->last_calculatedLiteralData = lineData.mid(STATISTICS_FIRST_USED_LABEL.length()).trimmed().split(" ")[0].toLong();
		}
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> item = getItemAndStoreTransferredBytes( lineData );
		if (!endReached && item.first != "") {
			item.first.prepend( "/" );
			this->progress_lastFilename = item.first;
			removeSymlinkString( &item.first );
			FileSystemUtils::convertToLocalPath( &item.first );
			uploadedItems << item;
			QString outputText;
			switch( item.second )
			{
				case UPLOADED:
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
			//qDebug() << outputText;
			//$$$ emit infoSignal( outputText );
		} else {
			emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
		}
	}
	qDebug() <<  QString::number( uploadedItems.size() ) << "files and/or directories processed";
	emit infoSignal( tr( "%1 files and/or directories processed" ).arg( uploadedItems.size() ) );
	waitForFinished();
	if (this->isAlive()) {
		QString standardErrors = readAllStandardError();
		if ( standardErrors != "" )
		{
			qWarning() << "Error occurred while uploading: " + standardErrors;
			if (errors) *errors = standardErrors;
		}
	} else {
		if ( this->exitCode() != 0)
		{
			throw ProcessException( QObject::tr( "rsync exited with with exitCode %1 (%2 %3).").arg(this->exitCode() ).arg(readAllStandardError().data()).arg(readAllStandardOutput().data()) );
		}
	}
	return uploadedItems;
}

QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > Rsync::upload( const QStringList& items, const QString& src, const QString& destination, const QStringList& includePatternList, const QStringList& excludePatternList, bool setDeleteFlag, bool compress, QString* errors, bool dry_run ) throw ( ProcessException )
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
	if (dry_run) { 	this->last_calculatedLiteralData = -1; arguments << "--stats" << "--only-write-batch=/dev/null"; }
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

		if ( Settings::IS_MAC )
		{
			QString outputString = item.normalized( QString::NormalizationForm_D );
			QByteArray output = outputString.toUtf8();
			write( output );
		}
		else if ( Settings::IS_WINDOWS )
		{
			write( item.toUtf8() );
		}
		else
		{
			write( item.toLocal8Bit() );
		}
		write( settings->getEOLCharacter() );
		waitForBytesWritten();
	}


	closeWriteChannel();

	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > uploadedItems;

	QString lineData;
	bool endReached = false;
	while( blockingReadLine( &lineData, 2147483647, 13 ) ) // -1 does not work on windows
	{
		qDebug() << lineData;
		lineData.replace( settings->getEOLCharacter(), "");
		endReached = endReached || lineData.contains(STATISTICS_FIRST_LABEL);
		if (lineData.contains(STATISTICS_FIRST_USED_LABEL)) {
			endReached = true; // hopefully not necessary
			this->last_calculatedLiteralData = lineData.mid(STATISTICS_FIRST_USED_LABEL.length()).trimmed().split(" ")[0].toLong();
		}
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> item = getItemAndStoreTransferredBytes( lineData );
		if (!endReached && item.first != "") {
			item.first.prepend( "/" );
			this->progress_lastFilename = item.first;
			removeSymlinkString( &item.first );
			FileSystemUtils::convertToLocalPath( &item.first );
			uploadedItems << item;
			QString outputText;
			switch( item.second )
			{
				case UPLOADED:
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
			qDebug() << outputText;
			emit infoSignal( outputText );
		} else {
			emit trafficInfoSignal( this->progress_lastFilename, this->progress_trafficB_s, this->progress_bytesRead, this->progress_bytesWritten );
		}
	}
	qDebug() <<  QString::number( uploadedItems.size() ) << "files and/or directories processed";
	emit infoSignal( tr( "%1 files and/or directories processed" ).arg( uploadedItems.size() ) );
	waitForFinished();
	if (this->isAlive()) {
		QString standardErrors = readAllStandardError();
		if ( standardErrors != "" )
		{
			qWarning() << "Error occurred while uploading: " + standardErrors;
			if (errors) *errors = standardErrors;
		}
	} else {
		if ( this->exitCode() != 0)
		{
			throw ProcessException( QObject::tr( "rsync exited with with exitCode %1 (%2 %3).").arg(this->exitCode() ).arg(readAllStandardError().data()).arg(readAllStandardOutput().data()) );
		}
	}
	return uploadedItems;
}


long Rsync::calculateUploadTransfer( const QHash<QString,bool> includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, QString* errors ) throw ( ProcessException )
{
	this->upload( includeRules, src, destination, setDeleteFlag, compress, errors, true );
	return this->last_calculatedLiteralData;
}


QStringList Rsync::downloadFullBackup( const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadFullBackup(" << backupName << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/*";
	return download( source, destination, false );
}

QStringList Rsync::downloadCustomBackup( const QString& backupName, const QStringList& itemList, const QString& destination )
{
	qDebug() << "Rsync::downloadCustomBackup(" << backupName << ", ..., " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/";
	return download( source, destination, itemList, false, true );
}

QStringList Rsync::downloadCustomBackup( const QString& backupName, const QHash<QString,bool>& selectionRules, const QString& destination )
{
	qDebug() << "Rsync::downloadCustomBackup(" << backupName << ", ..., " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/";
	return this->download( source, destination, selectionRules, false, true );
}

QFileInfo Rsync::downloadBackupContentFile( const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadBackupContentFile(" << backupName << ", " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + settings->getBackupPrefix() + "/" + settings->getMetaFolderName() + "/";

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

QFileInfo Rsync::downloadMetadata( const QString& backupName, const QString& destination )
{
	qDebug() << "Rsync::downloadMetadata(" << backupName << ", " << destination << ")";
	Settings* settings = Settings::getInstance();
	QString source = backupName + "/" + settings->getBackupPrefix() + "/" + settings->getMetaFolderName() + "/";

	downloadSingleFile( source, destination, settings->getMetadataFileName(), true, true );
	return destination + settings->getMetadataFileName();
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
		return destination + "/" + fileName.fileName();
	}
	return downloadedItems.at( 0 );
}

QStringList Rsync::download( const QString& source, const QString& destination, bool compress ) throw ( ProcessException )
{
	return download( source, destination, QStringList(), compress, true );
}

/**
 * deprecated
 */
QStringList Rsync::download( const QString& source, const QString& destination, const QStringList& customItemList, bool compress, bool emitErrorSignal ) throw ( ProcessException )
{
	qDebug() << "Rsync::download(" << source << ", " << destination << ", ... )";

	Settings* settings = Settings::getInstance();
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + source;

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
			if ( Settings::IS_WINDOWS )
			{
				write( item.toUtf8() );
			}
			else if ( Settings::IS_MAC )
			{
				QString outputString = item.normalized( QString::NormalizationForm_D );
				QByteArray output = outputString.toUtf8();
				write( output );
			}
			else
			{
				write( item.toLocal8Bit() );
			}
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
	while( blockingReadLine( &lineData, 2147483647, 13 ) ) // -1 does not work on windows
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

QStringList Rsync::download( const QString& source, const QString& destination, const QHash<QString,bool>& includeRules, bool compress, bool emitErrorSignal ) throw ( ProcessException )
{
	qDebug() << "Rsync::download(" << source << ", " << destination << ", ... )";

	Settings* settings = Settings::getInstance();
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + source;

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	arguments << getRsyncSshArguments();
	if (compress) arguments << "-z";
	arguments << QDir::cleanPath( src );
	arguments << getValidDestinationPath( destination );

	if ( includeRules.size() > 0 )
	{
		// read download list from input
		arguments << "--include-from=-";
		createProcess( settings->getRsyncName() , arguments );
		start();
		
		QList<QByteArray> convertedRules = calculateRsyncRulesFromIncludeRules(includeRules);
		foreach ( QByteArray rule, convertedRules )
		{
			write( rule );
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
	while( blockingReadLine( &lineData, 2147483647, 13 ) ) // -1 does not work on windows
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

QStringList Rsync::downloadAllRestoreInfoFiles( const QString& destination )
{
	qDebug() << "Rsync::downloadAllRestoreInfoFiles()";
	Settings* settings = Settings::getInstance();

	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":";

	QStringList arguments;
	arguments << getRsyncGeneralArguments();
	arguments << getRsyncDownloadArguments();
	arguments << getRsyncSshArguments();

	// include backup info file from backup folder
	QString includePath = settings->getBackupRootFolder();
	arguments << "--include=" + includePath;
	includePath += settings->getBackupPrefix() + "/";
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
	includePath += settings->getBackupPrefix() + "/";
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
	while( blockingReadLine( &lineData, 2147483647, 13 ) )
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
	QString src = settings->getServerUserName() + "@" + settings->getServerName() + ":" + settings->getBackupRootFolder();
	arguments << "--include=/*/";
	arguments << "--exclude=*";
	arguments << src;

	createProcess( settings->getRsyncName() , arguments );
	start();

	QStringList prefixes;
	QString lineData;
	while( blockingReadLine( &lineData, 2147483647, 13 ) )
	{
		lineData.replace( settings->getEOLCharacter(), "");

		QString column;
		QTextStream line( &lineData );
		line >> column; // skip permissions
		line >> column; // skip size
		line >> column; // skip modification date
		line >> column; // skip modification time
		line >> column; // take prefix
		if ( column != "." )
		{
			prefixes << column;
		}
	}
	waitForFinished();
	return prefixes;
}

QList<int> Rsync::getServerQuotaValues()
{
	qDebug() << "Rsync::getServerQuotaValues()";
	Settings* settings = Settings::getInstance();
	QString src = QFileInfo(settings->getAbsoluteBackupQuotaFileName()).absolutePath() + "/";
	QString dst = settings->getApplicationDataDir();
	QFileInfo quotaFileInfo = dst + this->downloadSingleFile( src, dst, QFileInfo(settings->getAbsoluteBackupQuotaFileName()).fileName(), true, true ).fileName();
	QList<int> sizes;
	if (quotaFileInfo.isFile()) {
 		QFile quotaFile(quotaFileInfo.absoluteFilePath());
		if (quotaFile.open(QIODevice::ReadOnly))
		{
			int quota, backup, snapshot;
			QTextStream in(&quotaFile);
			in >> quota >> backup >> snapshot;
			sizes.clear();
			sizes << quota << backup << snapshot;
		} else {
			sizes.clear();
		}
	} else {
		sizes.clear();
	}
	return sizes;
}


QStringList Rsync::getRsyncGeneralArguments()
{
	QStringList result;
	result << "--timeout=" + QString::number(Settings::getInstance()->getRsyncTimeout());
	if ( Settings::IS_WINDOWS )
	{
		result << "-iirtxS8";
	}
	else
	{
		result << "-iilrtxHS8";
		result << "--specials";
		result << "--progress";
	}	
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
		argument.append( settings->getSshName() );
		argument.append(" -i '" + settings->createPrivateOpenSshKeyFile() + "'");
		arguments << argument;
	}
	else
	{
		arguments << "'" + settings->getPlinkName() + "' -i " + "'" + settings->createPrivatePuttyKeyFile() + "'";
	}
	return arguments;
}

QString Rsync::getValidDestinationPath( const QString& destination )
{
	QString validDestination = destination;
	FileSystemUtils::convertToServerPath( &validDestination );
	if ( validDestination.size() > 1 && validDestination.endsWith( "/" ) )
	{
		return validDestination.left( validDestination.size() -1 );
	}
	return validDestination;
}

QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> Rsync::getItem( QString rsyncOutputLine )
{
	QTextStream textStream( &rsyncOutputLine, QIODevice::ReadOnly );
	QString flags;
	textStream >> flags;
	textStream.skipWhiteSpace();
	QString itemName = textStream.readAll();

	ITEMIZE_CHANGE_TYPE type;
	if( flags.startsWith( "<" ) )
	{
		type = UPLOADED;
	}
	else if( flags.startsWith( ">" ) )
	{
		type = DOWNLOADED;
	}
	else if( flags.startsWith( "*deleting" ) )
	{
		type = DELETED;
	}
	else
	{
		type = SKIPPED;
	}
	return qMakePair( itemName, type );
}

QList<QByteArray> Rsync::calculateRsyncRulesFromIncludeRules( const QHash<QString,bool> includeRules )
{
	QList<QByteArray> filters;
	QStack<QPair<QString,bool> > unclosedDirs;
	QString curDir = "/";
	QList<QString> includeRulesList = includeRules.keys();
	qSort(includeRulesList);
	QString lastRule = "";
	foreach (QString rule, includeRulesList)
	{
		bool ruleMod = includeRules[rule];
		QString ruleParentDir = StringUtils::parentDir(rule);
		QString ruleDir = StringUtils::dirPart(rule);
		bool isSubDir = (ruleDir!=curDir && ruleDir.startsWith(curDir));
		bool isParentDir = (ruleDir!=curDir && curDir.startsWith(ruleDir));
		qDebug() << "rule" << (ruleMod?"+":"-") << rule << "( ruleDir" << ruleDir << "curDir" << curDir << ")" << "isSubDir" << isSubDir << "isParentDir" << isParentDir;
		
		curDir = StringUtils::equalDirPart(curDir,ruleParentDir);
		while (unclosedDirs.size()>0 && !ruleDir.startsWith(unclosedDirs.top().first)) {
			QPair<QString,bool> dirToClose = unclosedDirs.pop();
			if (dirToClose.second || dirToClose.first !=  lastRule) { // don't exclude dir/** bejond dir/
				filters << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
				qDebug() << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
			} else {
				qDebug() << "dropped" << dirToClose.first;
			}
		}
		while (curDir!=ruleParentDir) {
			curDir = ruleDir.left(ruleDir.indexOf("/",curDir.length())+1); // next childDir
			if (curDir==StringUtils::dirPart(lastRule) && !includeRules[lastRule]) { // remove exclude before identical include
				filters.removeLast();
			}
			filters << convertRuleToByteArray( curDir,true );
			qDebug() << convertRuleToByteArray( curDir,true );
		}
		if (rule.endsWith("/")) { // rule is a dir
			filters << convertRuleToByteArray( ruleDir,ruleMod  );
			unclosedDirs.push( QPair<QString,bool>(ruleDir,ruleMod) );
			if (ruleMod) {
				curDir = ruleDir;
			}
		} else {
			filters << convertRuleToByteArray( rule,ruleMod );
			qDebug() << convertRuleToByteArray( rule,ruleMod );
		}		
		lastRule = rule;
	}
	while (unclosedDirs.size() > 0) {
		QPair<QString,bool> dirToClose = unclosedDirs.pop();
		if (dirToClose.second || dirToClose.first !=  lastRule) { // don't exclude dir/** bejond dir/
			filters << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
			qDebug() << convertRuleToByteArray( dirToClose.first + "**",dirToClose.second );
		}		
	}
	filters << convertRuleToByteArray( "**",false );
	qDebug() << "final filter list";
	foreach (QByteArray filter, filters)
		qDebug() << filter;
	return filters;
}

QByteArray Rsync::convertRuleToByteArray(QString rule, bool modifier)
{
	FileSystemUtils::convertToServerPath( &rule );
	QString rule_modifier = modifier ? "+ " : "- ";
	if ( Settings::IS_MAC )
		return (rule_modifier + rule.normalized( QString::NormalizationForm_D )).toUtf8();
	else if ( Settings::IS_WINDOWS )
		return (rule_modifier + rule).toUtf8();
	else
		return (rule_modifier + rule).toLocal8Bit();
}

void Rsync::testRsyncRulesConversion()
{
	qDebug() << "Rsync rules conversion test";
	
	QHash<QString,bool> includeRules;
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
		this->progress_trafficB_s = StringUtils::trafficStr2BytesPerSecond(parts[parts.size()-4]);
		this->progress_bytesRead = parts[parts.size()-2].toLongLong();
		this->progress_bytesWritten = parts[parts.size()-1].toLongLong();
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
	terminate();
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
	QStringList files;
	files << "/tmp2/";
	QString source = "/";
	QString destination = settings->getServerUserName() + "@" + settings->getServerName() + ":" + settings->getBackupRootFolder() + settings->getBackupPrefix() + "/" + settings->getBackupFolderName() + "/";
	Rsync rsync;
	QString errors;
	rsync.upload( files, source, destination, QStringList(), QStringList(), false, false, &errors );
	qDebug() << "errors: " << errors;
}

void Rsync::testDownloadCurrentMetadata()
{
	Rsync rsync;
	rsync.downloadCurrentMetadata( Settings::getInstance()->getApplicationDataDir(), false );
}

void Rsync::testDownloadAllRestoreInfoFiles()
{
	Rsync rsync;
	rsync.downloadAllRestoreInfoFiles( Settings::getInstance()->getApplicationDataDir() );
}

void Rsync::testDeleteAllRestoreInfoFiles()
{
	Rsync rsync;
	rsync.deleteAllRestoreInfoFiles( Settings::getInstance()->getApplicationDataDir() );
}

void Rsync::testDownloadBackupContentFile()
{
	Rsync rsync;
	rsync.downloadBackupContentFile( Settings::getInstance()->getBackupRootFolder(), Settings::getInstance()->getApplicationDataDir() );
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
