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

#ifndef RSYNC_HH
#define RSYNC_HH

#include <QString>
#include <QFileInfo>

#include "tools/abstract_rsync.hh"
#include "tools/process.hh"

/**
 * The Rsync class provides methods for using the rsync tool
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Rsync : public AbstractRsync, public Process
{
	Q_OBJECT

public slots:
	void abort();
	void bufferedInfoOutput();
private slots:
	void updateInfoOutput();

signals:
	void volumeCalculationInfoSignal(const QString& filename, long files_total, long files_done);
	void trafficInfoSignal(const QString& filename, float traffic, quint64 bytesRead, quint64 bytesWritten);

public:

	/**
	 * Constructs an Rsync
	 */
	Rsync();

	/**
	 * Destroys the Rsync
	 */
	virtual ~Rsync();

	/**
	 * @see AbstractRsync::upload( const QStringList& items, const QString& source, const QString& destination, const QString& includePattern, const QString& excludePattern, QString* errors )
	 */
	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > upload( const BackupSelectionHash& includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, QString* errors, QString* warnings, bool dry_run=false ) throw ( ProcessException );
	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > upload( const QStringList& items, const QString& source, const QString& destination, const QStringList& includePatternList, const QStringList& excludePatternList, bool setDeleteFlag, bool bCompress, QString* errors, bool dry_run=false ) throw ( ProcessException );

	/**
	 *
	 */
	long calculateUploadTransfer( const BackupSelectionHash includeRules, const QString& src, const QString& destination, bool setDeleteFlag, bool compress, QString* errors, QString* warnings ) throw ( ProcessException );

	/**
	 * @see AbstractRsync::downloadFullBackup( const QString& backup_prefix, const QString& backupName, const QString& destination )
	 */
	QStringList downloadFullBackup( const QString& backup_prefix, const QString& backupName, const QString& destination );

	/**
	 * @see AbstractRsync::downloadCustomBackup( const QString& backupName, const QStringList& itemList, const QString& destination )
	 */
	QStringList downloadCustomBackup( const QString& backup_prefix, const QString& backupName, const BackupSelectionHash& selectionRules, const QString& destination );
	QStringList downloadCustomBackup( const QString& backup_prefix, const QString& backupName, const QStringList& itemList, const QString& destination );

	/**
	 * @see AbstractRsync::downloadBackupContentFile( const QString& backupName, const QString& destination )
	 */
	QFileInfo downloadBackupContentFile( const QString& backup_prefix, const QString& backupName, const QString& destination );

	/**
	 * @see AbstractRsync::downloadCurrentBackupContentFile( const QString& destination )
	 */
	QFileInfo downloadCurrentBackupContentFile( const QString& destination, bool emitErrorSignal );

	/**
	 * @see AbstractRsync::downloadMetadata( const QString& backup_prefix, const QString& backupName, const QString& destination )
	 */
	QFileInfo downloadMetadata( const QString& backup_prefix, const QString& backupName, const QString& destination );

	/**
	 * @see AbstractRsync::downloadCurrentMetadata( const QString& destination )
	 */
	QFileInfo downloadCurrentMetadata( const QString& destination, bool emitErrorSignal = true );

	/**
	 * @see AbstractRsync::downloadAllRestoreInfoFiles( const QString& destination )
	 */
	QStringList downloadAllRestoreInfoFiles( const QString& destination, const QString& backup_prefix );

	/**
	 * @see AbstractRsync:deleteAllRestoreInfoFiles( const QString& destination )
	 */
	 void deleteAllRestoreInfoFiles( const QString& path );

	 /**
		* @see AbstractRsync::getPrefixes()
		*/
	 QStringList getPrefixes();

	 /**
	 * Tests the getPrefixes method
	 * @see getPrefixes()
	 */
	static void testGetPrefixes();

	/**
	 * Tests the backup method
	 * @see backup()
	 */
	static void testUpload();

	/**
	 * Tests the downloadCurrentMetadata method
	 * @see downloadCurrentMetadata()
	 */
	static void testDownloadCurrentMetadata();

	/**
	 * Tests the downloadAllRestoreInfoFiles method
	 * @see downloadAllRestoreInfoFiles()
	 */
	static void testDownloadAllRestoreInfoFiles();

	/**
	 * Tests the deleteAllRestoreInfoFiles method
	 * @see deleteAllRestoreInfoFiles()
	 */
	static void testDeleteAllRestoreInfoFiles();

	/**
	 * Tests the downloadBackupContentFile method
	 * @see downloadBackupContentFile()
	 */
	static void testDownloadBackupContentFile();

	/**
	 * Tests the private getItem method
	 */
	static void testGetItem();
	static void testRsyncRulesConversion();

private:
	static QStringList getRsyncGeneralArguments();
	static QStringList getRsyncUploadArguments();
	static QStringList getRsyncDownloadArguments();
	static QStringList getRsyncProgressArguments();
	static QStringList getRsyncSshArguments();

	static QByteArray convertFilenameForRsyncArgument(QString filename);
	static QString getValidDestinationPath( const QString& destination );
	static QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> getItem( QString rsyncOutputLine );
	QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> getItemAndStoreTransferredBytes( QString rsyncOutputLine );
	static QList<QByteArray> calculateRsyncRulesFromIncludeRules( const BackupSelectionHash& includeRules, QStringList* files_from_list = 0 );
	static void removeSymlinkString( QString* path );

	quint64 progress_bytesRead;
	quint64 progress_bytesWritten;
	float progress_trafficB_s;
	QString progress_lastFilename;
	quint64 last_calculatedLiteralData;

	long files_total;
	long cur_n_files_done;
	QDateTime lastUpdateTime;

	QFileInfo downloadSingleFile( const QString& source, const QString& destination, const QFileInfo& fileName, bool compress, bool emitErrorSignal );
	QStringList download( const QString& source, const QString& destination, bool compress) throw ( ProcessException );
	QStringList download( const QString& source, const QString& destination, const QStringList& customItemList, bool compress, bool emitErrorSignal ) throw ( ProcessException );
	QStringList download( const QString& source, const QString& destination, const BackupSelectionHash& includeRules, bool compress, bool emitErrorSignal ) throw ( ProcessException );

	static QFileInfo getWriteIncludeFileName(const BackupSelectionHash& includeRules);
	static QByteArray convertRuleToByteArray(QString rule, bool modifier );
	static QByteArray convertQStringToQByteArray(QString aStr);
};

#endif

