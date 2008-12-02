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

#ifndef MAIN_MODEL_HH
#define MAIN_MODEL_HH

#include <QDebug>
#include <QObject>
#include <QSettings>
#include <QDirModel>
#include <QStandardItemModel>
#include <QList>

#include "model/remote_dir_model.hh"
#include "model/restore_name.hh"
#include "settings/settings.hh"
#include "tools/abstract_rsync.hh"

/**
 * The MainModel class the main model according to the MVC pattern
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class MainModel : public QObject
{
	Q_OBJECT

private:
	static const QString SETTING_FILE_NAME;

	static const QString PLATFORM_SETTINGS;
	static const QString OPERATING_SYSTEM;

	static const QString USER_SETTINGS;
	static const QString SETTING_USERNAME;
	static const QString SETTING_PASSWORD;
	static const QString SETTING_EMAIL;
	static const QString SETTING_LANGUAGEID;

public:
	/**
	 * Date and time format for saving in backup metadata ( currently called backupTime )
	 * This file is changed on the server at the first rsnaphots backup
	 */
	static const QString BACKUP_TIME_FORMAT;

	/**
	 * String indicating the current backup
	 */
	static const QString BACKUP_TIME_TODAY;

	/**
	 * Constructs a MainModel
	 */
	MainModel();

	/**
	 * Destroys the MainModel
	 */
	virtual ~MainModel();

	/**
	 * After reinstalling the application, calling this method keeps the user's and application's settings
	 */
	void keepSettings();

	/**
	 * After reinstalling the application, calling this method resets the user's and application's settings
	 */
	void deleteSettings();

	/**
	 * Gets a model representing the local file system
	 * @return a local directory model
	 */
	QDirModel* getLocalDirModel();

	/**
	 * Gets a model representing the remote directory structure for a given backup name
	 * @param backupName name of the backup
	 * @return a remote directory model
	 */
	QStandardItemModel* getRemoteDirModel( const QString& backupName );

	/**
	 * Initializes the connection to the server
	 * @return true if the connection is established
	 */
	bool initConnection();

	/**
	 * Backups the given list of files
	 * @param items List of items to backup
	 * @param includePatternList Pattern list for include files
	 * @param excludePatternList Pattern list for exclude files
	 * @param setDeleteFlag indicates whether extraneous files should be deleted
	 * @param startInCurrentThread indicates whether the method runs in its own thread or not
	 */
	void backup( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const bool& setDeleteFlag, const bool& startInCurrentThread );

	/**
	 * Checks if a lock exists
	 * @return the state of the lock
	 */
	static bool isLocked();

	/**
	 * Creates a lock
	 */
	static void lock();

	/**
	 * Removes the lock
	 */
	static void unlock();

	/**
	 * Checks whether the scheduler supports running the task after booting or not
	 * @return flag whether scheduling on start is supported
	 */
	bool isSchedulingOnStartSupported();

	/**
	 * Schedules a backup job to run at a specific day and time
	 * @param items List of items to backup
	 * @param includePatternList Pattern list for include files
	 * @param excludePatternList Pattern list for exclude files
	 * @param time time to start the job
	 * @param days boolean array containing seven elements (days) with the value set to true for enabling the job at that day
	 * @param setDeleteFlag indicates whether extraneous files should be deleted
	 */
	void schedule( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const QTime& time, const bool days[], const bool& setDeleteFlag );

	/**
	 * Schedules a backup job to run after booting
	 * @param items List of items to backup
	 * @param includePatternList Pattern list for include files
	 * @param excludePatternList Pattern list for exclude files
	 * @param minutesToDelay minutes to delay before starting after booting
	 * @param setDeleteFlag indicates whether extraneous files should be deleted
	 */
	void schedule( const QStringList& items, const QStringList& includePatternList, const QStringList& excludePatternList, const int& minutesToDelay, const bool& setDeleteFlag );

	/**
	 * Gets all available restore names
	 * @return a list of RestoreName items
	 */
	QList<RestoreName> getRestoreNames();

	/**
	 * Gets all available prefixes from the server
	 * @return a list of prefix names
	 */
	QStringList getPrefixes();

	/**
	 * Restores all items
	 * @param backupName name of the backup for fetching the restore items
	 * @param destination a destination path for restoring
	 */
	void fullRestore( const QString& backupName, const QString& destination );

	/**
	 * Restores a custom list of items
	 * @param remoteDirModel remote dir model
	 * @param selectionList selected items
	 * @param backupName name of the backup for fetching the restore items
	 * @param destination a destination path for restoring
	 */
	void customRestore( const QStandardItemModel* remoteDirModel, const QModelIndexList selectionList, const QString& backupName, const QString& destination );

	/**
	 * Shows an information message box with the given text
	 * @param message text for the message box
	 */
	void showInformationMessage( const QString& message );

	/**
	 * Shows an error message box with the given text
	 * @param message text for the message box
	 */
	void showCriticalMessage( const QString& message );

	/**
	 * Sets the given message to the status bar
	 * @param message text for the status bar
	 */
	void setStatusBarMessage ( const QString& message );

	/**
	 * Gets new logfile lines
	 */
	QStringList getNewLogfileLines();

	/**
	 * Exits the application
	 */
	void exit();

signals:
	void askForServerPassword( const QString& username, bool isUsernameEditable, int* result = 0, const QString& = "" );
	void askForClientPassword( const QString& username, bool isUsernameEditable, int* result = 0, const QString& = "" );
	void showInformationMessageBox( const QString& message );
	void showCriticalMessageBox( const QString& message );
	void updateStatusBarMessage ( const QString& message );

	void showProgressDialog( const QString& dialogTitle );
	void appendInfoMessage( const QString& message );
	void appendErrorMessage( const QString& message );
	void finishProgressDialog();
	void closeProgressDialog();
	void abortProcess();

public slots:

	/**
	 * Appends the given info-text to the progress dialog box
	 * @param text the text to append
	 */
	void infoSlot( const QString& text );

	/**
	 * Appends the given error-text to the progress dialog box
	 * @param text the text to append
	 */
	void errorSlot( const QString& text );

	/**
	 * Shows a progress dialog box
	 * @param dialogTitle title of the dialog
	 */
	void showProgressDialogSlot( const QString& dialogTitle );

	/**
	 * Closes the progress dialog box
	 */
	void closeProgressDialogSlot();

	/**
	 * Shows an info dialog box
	 * @param text the text to display
	 */
	void infoDialog( const QString& text );

	/**
	 * Shows an error dialog box
	 * @param text the text to display
	 */
	void errorDialog( const QString& text );

	/**
	 * Aborts the login process
	 */
	void abortLogin();

	/**
	 * Aborts the current process
	 */
	 void abortProcessSlot();

private:
	QDirModel* localDirModel;
	bool isLoginAborted;
	QStringList getRestoreContent( const QString& backupName );
};

inline void MainModel::abortLogin()
{
	this->isLoginAborted = true;
}

inline void MainModel::abortProcessSlot()
{
	emit abortProcess();
}

#endif
