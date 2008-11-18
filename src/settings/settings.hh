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

#ifndef SETTINGS_HH
#define SETTINGS_HH

#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QSize>
#include <QPoint>

#define macro_param_to_string(aStr) # aStr

/**
 * The Settings class is used to store and read all settings for this application
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: dsydler $ $Date: 2008/08/04 12:21:42 $ $Revision: 1.60 $
 */
class Settings
{
public:	

	/**
	 * True if the current operating system is Unix
	 */
	static const bool IS_UNIX;

	/**
	 * True if the current operating system is Mac
	 */
	static const bool IS_MAC;

	/**
	 * True if the current operating system is windows
	 */
	static const bool IS_WINDOWS;
	
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
	void loadSettings( const QFileInfo& configFile, const QString& resellerAffix = "_reseller" );
	
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
	 * Gets a list of supported application languages
	 * @return list of languages
	 */
	QStringList getSupportedLanguages();
	
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
	 * Indicates whether a reseller was given in by the build system or not 
	 */
	bool isReseller();
	
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
	QString getAuthorizedKeyFolderName();
	QString getAuthorizedKeyFileName();

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
	QStringList getBackupList();
	QStringList getIncludePatternList();
	void saveIncludePatternList( const QStringList& includePatternList );
	QStringList getExcludePatternList();
	void saveExcludePatternList( const QStringList& excludePatternList );
	void saveBackupItemList( const QStringList& backupItems );
	QString getUserName();
	QString getPassword();
	int getLanguageIndex();
	void saveUserName( const QString& userName );
	void setPassword( const QString& password );
	void saveLanguageIndex( const int& languageIndex );
	QString getBackupPrefix();
	void saveBackupPrefix( const QString& backupPrefix );
	QString getServerKey();
	void saveServerKey( const QString& serverKey );
	void saveBackupSettings( const QStringList& backupList );
	QString getPrivatePuttyKey();
	void savePrivatePuttyKey( const QString& privateKey );
	QString getPrivateOpenSshKey();
	void savePrivateOpenSshKey( const QString& privateKey );
	int getSchedulerDelay();
	void saveSchedulerDelay( const int& minutes );
	void saveDeleteExtraneousItems( const bool& deleteExtraneousItems );
	bool getDeleteExtraneousItems();
	QSize getWindowSize();
	void saveWindowSize( QSize size );
	QPoint getWindowPosition();
	void saveWindowPosition( QPoint position );

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
	static const QString SETTINGS_AUTHORIZED_KEY_FOLDER_NAME;
	static const QString SETTINGS_AUTHORIZED_KEY_FILE_NAME;
	
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
	static const QString SETTINGS_BACKUP_LIST;
	static const QString SETTINGS_SERVER_KEY;
	static const QString SETTINGS_BACKUP_PREFIX;
	static const QString SETTINGS_USERNAME;
	static const QString SETTINGS_PASSWORD;
	static const QString SETTINGS_LANGUAGE;
	static const QString SETTINGS_PRIVATE_PUTTY_KEY;
	static const QString SETTINGS_PRIVATE_OPEN_SSH_KEY;
	static const QString SETTINGS_SCHEDULER_DELAY;
	static const QString SETTINGS_DELETE_EXTRANEOUS_ITEMS;
	static const QString SETTINGS_WINDOW_SIZE;
	static const QString SETTINGS_WINDOW_POSITION;
	
	// [Reseller]
	static const QString SETTINGS_GROUP_RESELLER;
	static const QString SETTINGS_RESELLER_ADDRESS;

	static const QString SETTINGS_EXECUTABLE_EXTENSION;
	static const bool IS_RESELLER;

	static Settings* instance;

	QSettings* applicationSettings; // read only settings
	QSettings* resellerSettings; // read only reseller settings
	QSettings* userSettings; // writable settings
	bool settingsChanged;
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
	QString authorizedKeyFolderName;
	QString authorizedKeyFileName;

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
	QString userName;
	int languageIndex;
	QString serverKey;
	QString privatePuttyKey;
	QString privateOpenSshKey;
	QStringList backupList;
	int schedulerDelay;
	bool deleteExtraneousItems;
	QSize windowSize;
	QPoint windowPosition;
	
	// [Reseller]
	QString resellerAddress;

	// non persistent
	QString password;
	
private:
	Settings();
	virtual ~Settings();
	void setCurrentOS( const QString& osName );
	bool createKeyFile( const QString& key, const QString& keyFilePath );

};

inline int Settings::getMaxLogLines()
{
	return maxLogLines;	
}

inline QString Settings::getDefaultServerName()
{
	return defaultServerName;	
}

inline QString Settings::getDefaultServerKey()
{
	return defaultServerKey;	
}

inline QString Settings::getServerName()
{
	return serverName;
}

inline bool Settings::getDeleteExtraneousItems()
{
	return deleteExtraneousItems;	
}

inline QStringList Settings::getSupportedLanguages()
{
	return supportedLanguages;	
}

inline int Settings::getSchedulerDelay()
{
	return schedulerDelay;
}

inline bool Settings::isLogDebugMessageEnabled()
{
	return logDebugMessage;
}

inline QString Settings::getPrivatePuttyKey()
{
	return privatePuttyKey;
}

inline QString Settings::getPrivateOpenSshKey()
{
	return privateOpenSshKey;
}

inline QString Settings::getBackupTimeFileName()
{
	return backupTimeFileName;
}

inline QString Settings::getLogFileName()
{
	return logFileName;
}

inline QString Settings::getApplicationBinDir()
{
	return applicationBinDir;
}

inline QString Settings::getApplicationDataDir()
{
	return applicationDataDir;
}


inline QString Settings::getSetAclName()
{
	return getApplicationBinDir() + setacl;
}

inline QString Settings::getLockFileName()
{
	return lockFileName;
}

inline QString Settings::getThisApplicationFullPathExecutable()
{
	return getApplicationBinDir() + thisApplication;	
}

inline QString Settings::getThisApplicationExecutable()
{
	return thisApplication;	
}

inline QStringList Settings::getBackupList()
{
	return backupList;
}

inline QString Settings::getPassword()
{
	return password;
}

inline QString Settings::getApplicationName()
{
	return applicationName;
}

inline QString Settings::getServerKey()
{
	return serverKey;
}

inline QString Settings::getGetfaclName()
{
	return getApplicationBinDir() + getfacl;
}

inline QString Settings::getSetfaclName()
{
	return getApplicationBinDir() + setfacl;
}

inline QString Settings::getBackupFolderName()
{
	return backupFolderName;
}

inline QString Settings::getMetaFolderName()
{
	return metaFolderName;
}

inline QString Settings::getBackupRootFolder()
{
	return backupRootFolder;
}

inline QString Settings::getRestoreRootFolder()
{
	return restoreRootFolder;
}

inline QString Settings::getAuthorizedKeyFolderName()
{
	return authorizedKeyFolderName;
}

inline QString Settings::getAuthorizedKeyFileName()
{
	return authorizedKeyFileName;
}

inline QString Settings::getRsyncName()
{
	return getApplicationBinDir() + rsync;
}

inline QString Settings::getPlinkName()
{
	return getApplicationBinDir() + plink;
}

inline QString Settings::getSshName()
{
	return getApplicationBinDir() + ssh;	
}

inline int Settings::getLanguageIndex()
{
	return languageIndex;
}

inline QString Settings::getUserName()
{
	return userName;
}

inline QString Settings::getBackupContentFileName()
{
	return backupContentFileName;	
}

inline QString Settings::getBackupPrefix()
{
	return backupPrefix;
}

inline QString Settings::getLogFileAbsolutePath()
{
	return this->getApplicationDataDir() + this->getLogFileName();
}

inline int Settings::getRsyncTimeout()
{
	return rsyncTimeout;	
}

inline QSize Settings::getWindowSize()
{
	return windowSize;	
}

inline QPoint Settings::getWindowPosition()
{
	return windowPosition;
}

inline bool Settings::useOpenSshInsteadOfPlinkForRsync()
{
	return Settings::IS_MAC  || Settings::IS_UNIX;	
}

inline bool Settings::isReseller()
{
	return Settings::IS_RESELLER;	
}

inline QString Settings::getResellerAddress()
{
	return resellerAddress;
}
#endif
