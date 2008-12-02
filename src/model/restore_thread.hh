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

#ifndef RESTORE_THREAD_HH
#define RESTORE_THREAD_HH

#include <QThread>
#include <QStringList>

#include "exception/abort_exception.hh"
#include "tools/abstract_rsync.hh"

using std::auto_ptr;

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
	RestoreThread( const QString& backupName, const QString& destination );

	/**
	 * Creates a RestoreThread
	 * @param backupName name of the backup for fetching the restore items
	 * @param items selected items
	 * @param destination a destination path for restoring
	 */
	RestoreThread( const QString& backupName, const QStringList& items, const QString& destination );

	/**
	 * Destroyes the RestoreThread
	 */
	virtual ~RestoreThread();

signals:
	void showCriticalMessageBox( const QString& message );
	void appendInfoMessage( const QString& message );
	void appendErrorMessage( const QString& message );
	void finishProgressDialog();
	void abort();

public slots:
	void abortRestoreProcess();

protected:

	/**
	 * Runs the restore process
	 */
	void run();

private:
	void init();
	void checkAbortState() throw ( AbortException );
	void applyMetadata( const QString& backupName, const QStringList& downloadedItems, const QString& downloadDestination );

	auto_ptr< AbstractRsync > rsync;
	bool isAborted;
	bool isCustomRestore;
	QString backupName;
	QStringList items;
	QString destination;
};

#endif
