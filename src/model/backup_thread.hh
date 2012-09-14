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

#ifndef BACKUP_THREAD_HH
#define BACKUP_THREAD_HH

#include <memory>
#include <QFileInfo>
#include <QList>
#include <QPair>
#include <QStringList>
#include <QThread>
#include <QDebug>

#include "tools/abstract_rsync.hh"
#include "exception/abort_exception.hh"
#include "model/backup_task.hh"
#include "utils/progress_task.hh"
#include "utils/string_utils.hh"

using std::auto_ptr;

/**
 * The BackupThread class runs the backup process in its own thread
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class BackupThread : public QThread
{
	Q_OBJECT

public:
	/**
	 * Creates a BackupThread
	 * @param items List of items to backup
	 * @param includePatternList Pattern list for include files
	 * @param excludePatternList Pattern list for exclude files
	 * @param setDeleteFlag indicates whether extraneous files should be deleted
	 */
	BackupThread( const BackupSelectionHash& includeRules );

	/**
	 * Destroys the BackupThread
	 */
	virtual ~BackupThread();

	void startInCurrentThread();
signals:
	void showCriticalMessageBox( const QString& message );
	void finishProgressDialog();
	void abort_rsync();
	void updateOverviewFormLastBackupsInfo();
	void infoSignal( const QString& text );
	void errorSignal( const QString& text );

	void progressSignal( const QString& taskText, float percentFinished, const QDateTime& timeRemaining, StringPairList infos = StringPairList());
	void finalStatusSignal( ConstUtils::StatusEnum status );

public slots:
	void rsyncDryrunProgressHandler(const QString&, long, long);
	void rsyncUploadProgressHandler(const QString& filename, float traffic, quint64 bytesRead, quint64 bytesWritten);
	void updateInformationToDisplay(StringPairList vars = StringPairList());
	void abortBackupProcess();

protected:

	/**
	 * Runs the backup process
	 */
	void run();

private:
	static const QString TASKNAME_PREPARE_DIRECTORIES;
	static const QString TASKNAME_ESTIMATE_BACKUP_SIZE;
	static const QString TASKNAME_FILE_UPLOAD;
	static const QString TASKNAME_DOWNLOAD_CURRENT_BACKUP_CONTENT;
	static const QString TASKNAME_UPLOAD_METADATA;
	static const QString TASKNAME_METAINFO;
	void checkAbortState();
	void prepareServerDirectories();
	quint64 estimateBackupSize( const QString& src, const QString& destination );
	void updateBackupContentFile( const QFileInfo& backupContentFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& backupList );
	QString createCurrentBackupTimeFile();
	void setLastBackupState(ConstUtils::StatusEnum status);
	ConstUtils::StatusEnum getLastBackupState();


	auto_ptr< AbstractRsync > rsync;
	bool isAborted;
	QStringList items;
	BackupSelectionHash includeRules;
	QStringList includePatternList;
	QStringList excludePatternList;
	bool setDeleteFlag;
	bool compressedUpload;
    int bandwidthLimit;
	QDateTime backupStartDateTime;
	ConstUtils::StatusEnum backupCurrentStatus;
	ProgressTask pt;
	int currentTaskNr;
};

#endif
