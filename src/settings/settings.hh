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

#ifndef SETTINGS_HH
#define SETTINGS_HH

#include <QObject>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>
#include <QSize>
#include <QPoint>

#include "config.h"

#include "model/backup_task.hh"
#include "model/scheduled_task.hh"
#include "utils/datatypes.hh"

// Forward declarations
class QSettings;

/**
 * The Settings class is used to store and read all settings for this application
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Settings : public QObject
{
		Q_OBJECT

	public:
		/**
		 * This flag is used to hide all progress information during backup and restore
		 */
		static const bool SHOW_PROGRESS;

		/**
		 * Gets a Settings instance
		 * @return a Settings instance
		 */
		static Settings* getInstance();

		/**
		 * Persists all member variables
		 */
		void saveSettings();

		/**
		 * Loads the user's and application's settings
		 * @param applicationName name of the application
		 * @param configFile the configuration file
		 */
		void loadSettings( const QFileInfo& configFile, const QString& resellerAffix = "_reseller", const QString& appDataAffix = "_appData" );

		/**
		 * Reloads all settings
		 */
		void reloadSettings();

		/**
		 * Gets the end of line character for the current operating system
		 * @return an end of line character
		 */
		const char* getEOLCharacter();

		/**
		 * Gets a list of available application languages
		 * @return list of languages
		 */
		QList<std::pair<QString, QString>> getAvailableLanguages();

		/**
		 * Creates a file containing the private putty key
		 * @return the private putty key file name
		 */
		QString createPrivatePuttyKeyFile();

		/**
		 * Creates a file containing the private openssh key
		 * @return the private openssh key file name
		 */
		QString createPrivateOpenSshKeyFile();

		/**
		 * Deletes the files containing the user's private keys
		 */
		void deletePrivateKeyFiles();

		/**
		 * Gets the host name
		 * @return name of the host
		 */
		QString getLocalHostName();

		/**
		 * Checks if the application has been reinstalled
		 * @return true if the application has been reinstalled, otherwise returns false
		 */
		bool isReinstalled();

		/**
		 * Checks if a computername (backup-prefix) and username is provided
		 * @return true if at least one of both settings (username or backup prefix) is missing
		 */
		bool isInevitableSettingsMissing();

		/**
		 * After reinstalling the application, calling this method keeps the user's and application's settings
		 */
		void keepSettings();

		/**
		 * After reinstalling the application, calling this method resets the user's and application's settings
		 */
		void deleteSettings();

		/**
		 * Gets the name of the application. This property is read-only.
		 * @return the application's name
		 */
		QString getApplicationName();

		/**
		 * Gets the name of the lock file. This property is read-only.
		 * @return the lock file's name
		 */
		QString getLockFileName();

		/**
		 * Gets the name of the log file. This property is read-only.
		 * @return the log file's name
		 */
		QString getLogFileName();

		/**
		 * Gets the absolute path to the log file. This property is read-only.
		 * @return the log file's absolute path
		 */
		QString getLogFileAbsolutePath();

		/**
		 * Checks if logging of debug messages is enabled. This property is read-only.
		 * @return true if enabled, otherwise returns false
		 */
		bool isLogDebugMessageEnabled();

		/**
		 * Gets the maximum number of log lines. This property is read-only.
		 * @return number of log lines
		 */
		int getMaxLogLines();

		/**
		 * Gets the timeout value in seconds for rsync. This property is read-only.
		 * @return number of seconds to timeout
		 */
		int getRsyncTimeout();

		/**
		 * Indicates whether to use the openssh client instead of plink for rsync
		 * This is a workaround for rsync-plink deadlock problem
		 * (see bug 854 https://old-bugzilla.puzzle.ch/show_bug.cgi?id=854 for a detail description)
		 */
		bool useOpenSshInsteadOfPlinkForRsync();

		/**
		 * returns the provided reseller address from config_reseller
		 */
		QString getResellerAddress();


		QString getDefaultServerName();
		QString getDefaultServerKey();
		QString getBackupFolderName();
		QString getMetaFolderName();
		QString getBackupRootFolder();
		QString getRestoreRootFolder();
		QString getMetadataFileName();
		QString getTempMetadataFileName();
		QString getBackupContentFileName();
		QString getBackupTimeFileName();
		QString getServerQuotaScriptName();
		QString getAuthorizedKeyFolderName();
		QString getAuthorizedKeyFileName();
		QString getQuotaModificationUrl();
		QString getQuotaModificationUrlUidParam();

		QString getThisApplicationFullPathExecutable();
		QString getThisApplicationExecutable();
		QString getRsyncName();
		QString getPlinkName();
		QString getSshName();
		QString getGetfaclName();
		QString getSetfaclName();
		QString getSetAclName();

		QString getApplicationBinDir();
		QString getApplicationDataDir();

		// writable settings
		QString getServerName();
		void saveServerName( const QString& serverName );
		void saveInstallDate( const QDateTime& installDate, bool force_write = false );
		QDateTime getInstallDate();
		void saveBackupItemList( const QStringList& backupItems );
		QString getClientUserName();
		QString getClientPassword();
		QString getServerUserName();
		QString getServerPassword();
		QString getLanguage() const;
		void saveServerUserName( const QString& userName );
		void setServerPassword( const QString& password );
		void setClientPassword( const QString& password );
		void saveLanguage( const QString& language );
		QString getBackupPrefix();
		void saveBackupPrefix( const QString& backupPrefix );
		QString getServerKey();
		void saveServerKey( const QString& serverKey );
		void saveBackupSettings( const QStringList& backupList );
		QString getPrivatePuttyKey();
		void savePrivatePuttyKey( const QString& privateKey );
		QString getPrivateOpenSshKey();
		void savePrivateOpenSshKey( const QString& privateKey );
		//int getSchedulerDelay();
		//void saveSchedulerDelay( const int& minutes );
		void saveDeleteExtraneousItems( const bool& deleteExtraneousItems );
		bool getDeleteExtraneousItems();
		void saveShowHiddenFilesAndFolders( const bool showHiddenFilesAndFolders );
		bool getShowHiddenFilesAndFolders();
		bool isCompressedRsyncTraffic();
		QSize getWindowSize();
		void saveWindowSize( QSize size );
		QPoint getWindowPosition();
		void saveWindowPosition( QPoint position );
        void saveLogDebugMessages(bool logDebugMessage);
        bool getLogDebugMessages() const;
        void saveBandwidthLimit(int bandwidthLimit);
        int getBandwidthLimit() const;

		int getNOfLastBackups() const;
		void saveNOfLastBackups( int nOfLastBackups );
		const QList<BackupTask>& getLastBackups() const;
		void addLastBackup( const BackupTask& lastBackup );
		void replaceLastBackup( const BackupTask& newBackupInfo );
		void saveLastBackups();
		const ScheduledTask& getScheduleRule() const;
		void saveScheduleRule( const ScheduledTask& scheduleRule );
		const BackupSelectionHash& getLastBackupSelectionRules() const;
		void saveBackupSelectionRules( const BackupSelectionHash& selectionRules );

		static const QString VERSION;

	private:
		static const QString EXECUTABLE_NAME;

		// [Application]
		static const QString SETTINGS_GROUP_APPLICATION;

		// [Client]
		static const QString SETTINGS_GROUP_CLIENT;
		static const QString SETTINGS_APPLICATION_FULL_NAME;
		static const QString SETTINGS_PRIVATE_PUTTY_KEY_FILE_NAME;
		static const QString SETTINGS_PRIVATE_OPEN_SSH_KEY_FILE_NAME;
		static const QString SETTINGS_LOCK_FILE_NAME;
		static const QString SETTINGS_LOG_FILE_NAME;
		static const QString SETTINGS_LOG_DEBUG_MESSAGE;
		static const QString SETTINGS_IGNORE_REINSTALL;
		static const QString SETTINGS_INCLUDE_PATTERN_FILE_NAME;
		static const QString SETTINGS_EXCLUDE_PATTERN_FILE_NAME;
		static const QString SETTINGS_MAX_LOG_LINES;
		static const QString SETTINGS_RSYNC_TIMEOUT;

		// [Server]
		static const QString SETTINGS_GROUP_SERVER;
		static const QString SETTINGS_HOST;
		static const QString SETTINGS_BACKUP_FOLDER_NAME;
		static const QString SETTINGS_META_FOLDER_NAME;
		static const QString SETTINGS_BACKUP_ROOT_FOLDER;
		static const QString SETTINGS_RESTORE_ROOT_FOLDER;
		static const QString SETTINGS_METADATA_FILE_NAME;
		static const QString SETTINGS_BACKUP_CONTENT_FILE_NAME;
		static const QString SETTINGS_BACKUP_TIME_FILE_NAME;
		static const QString SETTINGS_SERVER_QUOTA_SCRIPT_NAME;
		static const QString SETTINGS_AUTHORIZED_KEY_FOLDER_NAME;
		static const QString SETTINGS_AUTHORIZED_KEY_FILE_NAME;
		static const QString SETTINGS_QUOTA_MODIFICATION_URL;
		static const QString SETTINGS_QUOTA_MODIFICATION_URL_UID_PARAM;

		// [Executables]
		static const QString SETTINGS_GROUP_EXECUTABLES;
		static const QString SETTINGS_RSYNC;
		static const QString SETTINGS_PLINK;
		static const QString SETTINGS_SSH;
		static const QString SETTINGS_GETFACL;
		static const QString SETTINGS_SETFACL;
		static const QString SETTINGS_SETACL;

		// writable settings
		static const QString SETTINGS_INSTALL_DATE;
		static const QString SETTINGS_BACKUP_RULES;
		static const QString SETTINGS_SERVER_KEY;
		static const QString SETTINGS_BACKUP_PREFIX;
		static const QString SETTINGS_USERNAME;
		static const QString SETTINGS_PASSWORD;
		static const QString SETTINGS_LANGUAGE;
		static const QString SETTINGS_PRIVATE_PUTTY_KEY;
		static const QString SETTINGS_PRIVATE_OPEN_SSH_KEY;
		static const QString SETTINGS_WINDOW_SIZE;
		static const QString SETTINGS_WINDOW_POSITION;
		static const QString SETTINGS_DELETE_EXTRANEOUS_ITEMS;
		static const QString SETTINGS_SHOW_HIDDEN_FILES_AND_FOLDERS;
        static const QString SETTINGS_BANDWITH_LIMIT;

		// [Reseller]
		static const QString SETTINGS_GROUP_RESELLER;
		static const QString SETTINGS_RESELLER_ADDRESS;

		// [GUI]
		static const QString SETTINGS_GROUP_GUI;
		static const QString SETTINGS_N_OF_SHOWN_LAST_BACKUPS;

		// [ApplicationData] only saved from session to session, not manually editable settings
		static const QString SETTINGS_GROUP_APPDATA;
		static const QString SETTINGS_APPDATA_LASTBACKUPS;
		static const QString SETTINGS_APPDATA_LASTBACKUP_DATE;
		static const QString SETTINGS_APPDATA_LASTBACKUP_STATUS;
		static const QString SETTINGS_SCHEDULE_RULE;
		static const QString SETTINGS_LAST_BACKUP_RULES;
		static const QString SETTINGS_LAST_BACKUP_RULES_ITEM;
		static const QString SETTINGS_LAST_BACKUP_RULES_MODIFIER;


		// Settings given at build-time or by platform
		static Settings* instance;

		QSettings* applicationSettings; // read only settings
		QSettings* resellerSettings; // read only reseller settings
		QSettings* userSettings; // writable settings
		QSettings* appData; // writable settings
		QString applicationBinDir;
		QString applicationDataDir;
		QDateTime installDate;
		QStringList supportedLanguages;

		// [Client]
		QString applicationName;
		QString privatePuttyKeyFileName;
		QString privateOpenSshKeyFileName;
		QString includePatternFileName;
		QString excludePatternFileName;
		QString backupPrefix;
		QString lockFileName;
		QString logFileName;
		bool logDebugMessage;
		bool ignoreReinstall;
		int maxLogLines;
		int rsyncTimeout;

		// [Server]
		QString defaultServerName;
		QString defaultServerKey;
		QString backupFolderName;
		QString metaFolderName;
		QString backupRootFolder;
		QString restoreRootFolder;
		QString metadataFileName;
		QString backupContentFileName;
		QString backupTimeFileName;
		QString serverQuotaScriptName;

		QString authorizedKeyFolderName;
		QString authorizedKeyFileName;
		QString quotaModificationUrl;
		QString quotaModificationUrlUidParam;

		// [Executables]
		QString thisApplication;
		QString rsync;
		QString plink;
		QString ssh;
		QString getfacl;
		QString setfacl;
		QString setacl;

		// writable
		QString serverName;
		QString serverUserName;
		QString language;
		QString serverKey;
		QString privatePuttyKey;
		QString privateOpenSshKey;
		bool deleteExtraneousItems;
		bool showHiddenFilesAndFolders;
        int bandwidthLimit;
		QSize windowSize;
		QPoint windowPosition;

		// [Reseller]
		QString resellerAddress;

		// [GUI]
		int nOfLastBackups;
		static const int MAX_SAVED_LAST_BACKUPS; // immutable
		static const int DEFAULT_NUM_OF_LAST_BACKUPS; // immutable

		// [AppData]
		QList<BackupTask> lastBackups;
		ScheduledTask scheduleRule;
		BackupSelectionHash lastBackupRules;

		// non persistent
		QString client_password;
		QString server_password;

		int effectiveUserId;

	private slots:
		void setServerUserNameAndPassword( const QString& userName, const QString& password, const bool isUsernameEditable );
		void setClientUserNameAndPassword( const QString& userName, const QString& password, const bool isUsernameEditable );


	private:
		Settings();
		virtual ~Settings();
		bool createKeyFile( const QString& key, const QString& keyFilePath );

};
#endif
