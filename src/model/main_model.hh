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

#ifndef MAIN_MODEL_HH
#define MAIN_MODEL_HH

#include <QObject>
#include <QDirModel>
#include <QStandardItemModel>
#include <QList>

#include "model/restore_name.hh"
#include "model/scheduled_task.hh"
#include "tools/abstract_informing_process.hh"

// Forward declarations
class LocalDirModel;
class RemoteDirModel;
class SpaceUsageModel;
class FilesystemSnapshot;
class BackupThread;

/**
 * The MainModel class the main model according to the MVC pattern
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class MainModel : public AbstractInformingProcess
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
	LocalDirModel* getLocalDirModel();

	/**
	 * Gets a model representing the remote directory structure for a given backup name
	 * @param backupName name of the backup
	 * @return a remote directory model
	 */
	QStandardItemModel* getCurrentRemoteDirModel_general();
	RemoteDirModel* getCurrentRemoteDirModel();
	RemoteDirModel* loadRemoteDirModel( const QString& backup_prefix, const QString& backupName );
	void clearRemoteDirModel();

    /**
     * Get a pointer to the space usage model.
     * Not const due to Qt signal/slot system.
     */
    const SpaceUsageModel* getSpaceUsageModel();

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
	void backup( const BackupSelectionHash& includeRules, const bool& startInCurrentThread );

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
	 * more general schedule method, based on ScheduleTask
	 * @param items List of items to backup
	 * @param includePatternList Pattern list for include files
	 * @param excludePatternList Pattern list for exclude files
	 * @param scheduleRule ScheduledTask-object indicating type and information for scheduling
	 * @param setDeleteFlag indicates whether extraneous files should be deleted
	 */
	void schedule( const BackupSelectionHash& includeRules, const ScheduledTask& scheduleRule );

	/**
	 * Gets all available restore names
	 * @return a list of RestoreName items
	 */
	QList<RestoreName> getRestoreNames( const QString & backup_prefix );

	/**
	 * Gets all available prefixes from the server
	 * @return a list of prefix names
	 */
	QStringList getPrefixes();

	 /**
	 * Reads int-values from the provided quota-file on the server
	 * @return QList<int> of values from the quota-file
	  */
	QList<int> getServerQuota();

	/**
	 * Restores all items
	 * @param backupName name of the backup for fetching the restore items
	 * @param destination a destination path for restoring
	 */
	void fullRestore( const QString& backup_prefix, const QString& backupName, const QString& destination );

	/**
	 * Restores a custom list of items
	 * @param selectionList selected items
	 * @param backupName name of the backup for fetching the restore items
	 * @param destination a destination path for restoring
	 */
	void customRestore( const BackupSelectionHash& selectionRules, const QString& backup_prefix, const QString& backupName, const QString& destination );

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
	void updateOverviewFormLastBackupsInfo();

	void showProgressDialog( const QString& dialogTitle );
	void finishProgressDialog();
	void closeProgressDialog();
	bool abortProcess();

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
	bool abortProcessSlot();

	/**
	 * Aborts the current process
	 */
	void uploadFiles();

private:
	LocalDirModel* localDirModel;
	RemoteDirModel* remoteDirModel;
    SpaceUsageModel* spaceUsageModel;
	bool isLoginAborted;
	QStringList getRestoreContent( const QString& backup_prefix, const QString& backupName );
	FilesystemSnapshot* fsSnapshot;
	BackupThread* backupThread;
	bool startInThisThread;
};

inline void MainModel::abortLogin()
{
	this->isLoginAborted = true;
}

inline bool MainModel::abortProcessSlot()
{
	qDebug() << "MainModel::abortProcessSlot()";
	return(emit abortProcess());
}

#endif
