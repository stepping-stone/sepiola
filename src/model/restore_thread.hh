/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
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

#ifndef RESTORE_THREAD_HH
#define RESTORE_THREAD_HH

#include <memory>
#include <QThread>
#include <QStringList>

#include "utils/const_utils.hh"
#include "utils/datatypes.hh"

// Forward declarations
class AbstractRsync;

/**
 * The RestoreThread class runs the restore process in its own thread
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class RestoreThread : public QThread
{
	Q_OBJECT

public:
	/**
	 * Creates a RestoreThread
	 * @param backupName name of the backup for fetching the restore items
	 * @param destination a destination path for restoring
	 */
	RestoreThread( const QString& backup_prefix, const QString& backupName, const BackupSelectionHash& selectionRules, const QString& destination );

	/**
	 * Destroyes the RestoreThread
	 */
	virtual ~RestoreThread();


signals:
	void showCriticalMessageBox( const QString& message );
	void finishProgressDialog();
	void abort();
	void infoSignal( const QString& text );
	void errorSignal( const QString& text );

	void progressSignal( const QString& taskText, float percentFinished, const QDateTime& timeRemaining, StringPairList infos = StringPairList());
	void finalStatusSignal( ConstUtils::StatusEnum status );

public slots:
	void abortRestoreProcess();

protected:

	/**
	 * Runs the restore process
	 */
	void run();

private:
	void init();
	void checkAbortState();
	void applyMetadata( const QString& backup_prefix, const QString& backupName, const QStringList& downloadedItems, const QString& downloadDestination );
	void pushStateEvent(ConstUtils::StatusEnum eventState);

    std::shared_ptr<AbstractRsync> rsync;
	bool isAborted;
	bool isCustomRestore;
	QString backup_prefix;
	QString backupName;
	QStringList items;
	BackupSelectionHash selectionRules;
	QString destination;
	ConstUtils::StatusEnum restoreState; // class should be named TimedTask, but was once created only for backup_thread
};

#endif
