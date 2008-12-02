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
	BackupThread( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const bool& setDeleteFlag );

	/**
	 * Destroys the BackupThread
	 */
	virtual ~BackupThread();

	void startInCurrentThread();

signals:
	void showCriticalMessageBox( const QString& message );
	void appendInfoMessage( const QString& message );
	void appendErrorMessage( const QString& message );
	void finishProgressDialog();
	void abort();

public slots:
	void abortBackupProcess();

protected:

	/**
	 * Runs the backup process
	 */
	void run();

private:
	void checkAbortState() throw ( AbortException );
	void prepareServerDirectories();
	void updateBackupContentFile( const QFileInfo& backupContentFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& backupList );
	QString createCurrentBackupTimeFile();

	auto_ptr< AbstractRsync > rsync;
	bool isAborted;
	QStringList items;
	QStringList includePatternList;
	QStringList excludePatternList;
	bool setDeleteFlag;
};

#endif
