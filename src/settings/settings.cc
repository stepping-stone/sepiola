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

#include <QSettings>
#include <QObject>
#include <QHostInfo>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QLocale>
#include <stdio.h>

#include "settings/settings.hh"
#include "utils/file_system_utils.hh"
#include "utils/string_utils.hh"
#include "utils/log_file_utils.hh"
#include "utils/datatypes.hh"
#include "model/scheduled_task.hh"

#ifdef Q_OS_UNIX
const bool Settings::IS_UNIX = true;
#else
const bool Settings::IS_UNIX = false;
#endif

#ifdef Q_OS_WIN32
const bool Settings::IS_WINDOWS = true;
const QString Settings::SETTINGS_EXECUTABLE_EXTENSION = ".exe";
#else
const bool Settings::IS_WINDOWS = false;
const QString Settings::SETTINGS_EXECUTABLE_EXTENSION = "";
#endif

#ifdef Q_OS_MAC
const bool Settings::IS_MAC = true;
#else
const bool Settings::IS_MAC = false;
#endif

const QString Settings::EXECUTABLE_NAME = QString( SSBACKUP_EXECUTABLE_NAME );
const QString Settings::VERSION = QString::number( CPACK_PACKAGE_VERSION_MAJOR ) + "." + QString::number( CPACK_PACKAGE_VERSION_MINOR ) + "." + QString::number( CPACK_PACKAGE_VERSION_PATCH );
const bool Settings::IS_RESELLER = ( CPACK_IS_RESELLER == 1 ? true : false );

// [Application]
const QString Settings::SETTINGS_GROUP_APPLICATION = "Application";

// [Client]
const QString Settings::SETTINGS_GROUP_CLIENT = "Client";
const QString Settings::SETTINGS_APPLICATION_FULL_NAME = "ApplicationName";
const QString Settings::SETTINGS_PRIVATE_PUTTY_KEY_FILE_NAME = "PrivateKeyFileName";
const QString Settings::SETTINGS_PRIVATE_OPEN_SSH_KEY_FILE_NAME = "PrivateOpenSshKeyFileName";
const QString Settings::SETTINGS_LOCK_FILE_NAME = "LockFileName";
const QString Settings::SETTINGS_LOG_FILE_NAME = "LogFileName";
const QString Settings::SETTINGS_LOG_DEBUG_MESSAGE = "LogDebugMessage";
const QString Settings::SETTINGS_INCLUDE_PATTERN_FILE_NAME = "InlcudePatternFileName";
const QString Settings::SETTINGS_EXCLUDE_PATTERN_FILE_NAME = "ExcludePatternFileName";
const QString Settings::SETTINGS_MAX_LOG_LINES = "MaxLogLines";
const QString Settings::SETTINGS_RSYNC_TIMEOUT = "RsyncTimeout";

// [Server]
const QString Settings::SETTINGS_GROUP_SERVER = "Server";
const QString Settings::SETTINGS_HOST = "Host";
const QString Settings::SETTINGS_BACKUP_FOLDER_NAME = "BackupFolderName";
const QString Settings::SETTINGS_META_FOLDER_NAME = "MetaFolderName";
const QString Settings::SETTINGS_BACKUP_ROOT_FOLDER = "BackupRootFolder";
const QString Settings::SETTINGS_RESTORE_ROOT_FOLDER = "RestoreRootFolder";
const QString Settings::SETTINGS_METADATA_FILE_NAME = "MetadataFileName";
const QString Settings::SETTINGS_BACKUP_CONTENT_FILE_NAME = "BackupContentFileName";
const QString Settings::SETTINGS_BACKUP_TIME_FILE_NAME = "BackupTimeFileName";
const QString Settings::SETTINGS_SERVER_QUOTA_SCRIPT_NAME = "QuotaScript";
const QString Settings::SETTINGS_AUTHORIZED_KEY_FOLDER_NAME = "AuthorizedKeyFolderName";
const QString Settings::SETTINGS_AUTHORIZED_KEY_FILE_NAME = "AuthorizedKeyFileName";
const QString Settings::SETTINGS_QUOTA_MODIFICATION_URL = "QuotaModificationUrl";
const QString Settings::SETTINGS_QUOTA_MODIFICATION_URL_UID_PARAM = "QuotaModificationUrlUidParam";

// [Executables]
const QString Settings::SETTINGS_GROUP_EXECUTABLES = "Executables";
const QString Settings::SETTINGS_RSYNC = "Rsync";
const QString Settings::SETTINGS_PLINK = "Plink";
const QString Settings::SETTINGS_SSH = "Ssh";
const QString Settings::SETTINGS_GETFACL = "Getfacl";
const QString Settings::SETTINGS_SETFACL = "Setfacl";
const QString Settings::SETTINGS_SETACL = "Setacl";

// writable
const QString Settings::SETTINGS_INSTALL_DATE = "InstallDate";
const QString Settings::SETTINGS_USERNAME = "Username";
const QString Settings::SETTINGS_LANGUAGE = "Language";
const QString Settings::SETTINGS_BACKUP_PREFIX = "BackupPrefix";
const QString Settings::SETTINGS_BACKUP_RULES = "BackupRules";
const QString Settings::SETTINGS_SERVER_KEY = "ServerKey";
const QString Settings::SETTINGS_PRIVATE_PUTTY_KEY = "PrivateKey";
const QString Settings::SETTINGS_PRIVATE_OPEN_SSH_KEY = "PrivateOpenSshKey";
const QString Settings::SETTINGS_WINDOW_POSITION = "WindowPosition";
const QString Settings::SETTINGS_WINDOW_SIZE = "WindowSize";

// [Reseller]
const QString Settings::SETTINGS_GROUP_RESELLER = "Reseller";
const QString Settings::SETTINGS_RESELLER_ADDRESS = "ResellerAddress";

// [GUI]
const QString Settings::SETTINGS_GROUP_GUI = "GUI";
const QString Settings::SETTINGS_N_OF_SHOWN_LAST_BACKUPS = "nShownLastBackups";

// [AppData]
const QString Settings::SETTINGS_GROUP_APPDATA = "AppData";
const QString Settings::SETTINGS_APPDATA_LASTBACKUPS = "LastBackups";
const QString Settings::SETTINGS_APPDATA_LASTBACKUP_DATE = "LastBackupDate";
const QString Settings::SETTINGS_APPDATA_LASTBACKUP_STATUS = "LastBackupStatus";
const QString Settings::SETTINGS_SCHEDULE_RULE = "ScheduleRule";
const QString Settings::SETTINGS_LAST_BACKUP_RULES = "LastBackupRules";
const QString Settings::SETTINGS_LAST_BACKUP_RULES_ITEM = "Item";
const QString Settings::SETTINGS_LAST_BACKUP_RULES_MODIFIER = "Modifier";


// immutable
const int Settings::MAX_SAVED_LAST_BACKUPS = 10;
const int Settings::DEFAULT_NUM_OF_LAST_BACKUPS = 3;

Settings* Settings::instance = 0;

Settings::Settings()
{
	//TODO: load language name translations after setting the current langauge
	this->applicationSettings = 0;
	this->resellerSettings = 0;
	this->userSettings = 0;
	this->appData = 0;
#ifdef Q_OS_UNIX
	this->effectiveUserId = getuid();
#else
	this->effectiveUserId = 0;
#endif

	supportedLanguages << QObject::tr( "English" ); // default language has to be at the top (position zero)
	supportedLanguages << QObject::tr( "German" );
	qRegisterMetaType<BackupSelectionHash>("BackupSelectionHash");
	qRegisterMetaType<ScheduledTask>("ScheduledTask");
	qRegisterMetaType<StringPairList>("StringPairList");
	qRegisterMetaType<ConstUtils::StatusEnum>("ConstUtils::StatusEnum");
//	qRegisterMetaType<ScheduleRule::ScheduleType>("ScheduleRule::ScheduleType");
//	qRegisterMetaType<ScheduleRule::Weekdays>("ScheduleRule::Weekdays");
	qRegisterMetaTypeStreamOperators<ScheduledTask>("ScheduledTask");
}

Settings::~Settings()
{
}

Settings* Settings::getInstance()
{
	if ( !instance )
	{
		instance = new Settings;
	}
	return instance;
}

void Settings::loadSettings( const QFileInfo& configFile, const QString& resellerAffix, const QString& appDataAffix )
{
	qDebug() << "Settings::loadSettings: Loading settings from file <" << configFile.filePath() << ">";
	QDir homeDir = QDir::home();
	QString applicationDataDirName = "." + EXECUTABLE_NAME;
	QString resellerSettingsFileName = configFile.absoluteFilePath() + resellerAffix;
	if ( !homeDir.exists( applicationDataDirName ) )
	{
		homeDir.mkdir( applicationDataDirName );
	}
	applicationDataDir = homeDir.absolutePath() + "/" + applicationDataDirName + "/";
	QString appDataSettingsFileName  = applicationDataDir + configFile.fileName() + appDataAffix;
	applicationBinDir = configFile.absolutePath() + "/";
	qRegisterMetaType<ScheduledTask>("ScheduledTask");
	if (applicationSettings != 0) { delete applicationSettings; }
	applicationSettings = new QSettings( configFile.absoluteFilePath(), QSettings::IniFormat );
	if (resellerSettings != 0)    { delete resellerSettings; }
	resellerSettings =    new QSettings( resellerSettingsFileName, QSettings::IniFormat );
	if (userSettings != 0)        { delete userSettings; }
	userSettings =        new QSettings( applicationDataDir + configFile.fileName(), QSettings::IniFormat );
	if (appData != 0)             { delete appData; }
	appData =             new QSettings( appDataSettingsFileName, QSettings::IniFormat );

	if ( QFile( resellerSettingsFileName ).exists() )
	{
		// qDebug() << "Loading reseller-settings from file <" + resellerSettings->fileName() + ">";
	}
	else
	{
		// qDebug() << "Information: No reseller-specific settings provided <" + resellerSettingsFileName + "> -> settings are taken from standard config file <" + applicationSettings->fileName() + ">.";
	}

	reloadSettings();
	installDate = userSettings->value( SETTINGS_INSTALL_DATE ).toDateTime();
	if ( installDate.isNull() )
	{
		saveInstallDate( getInstallDate() );
	}
}

void Settings::reloadSettings()
{
	// [Client]
	qDebug() << "Settings::reloadSettings()";
	applicationSettings->beginGroup( SETTINGS_GROUP_CLIENT );
	applicationName = applicationSettings->value( SETTINGS_APPLICATION_FULL_NAME ).toString();
	logFileName = applicationSettings->value( SETTINGS_LOG_FILE_NAME ).toString();
	privatePuttyKeyFileName = applicationSettings->value( SETTINGS_PRIVATE_PUTTY_KEY_FILE_NAME ).toString();
	privateOpenSshKeyFileName = applicationSettings->value( SETTINGS_PRIVATE_OPEN_SSH_KEY_FILE_NAME ).toString();
	lockFileName = applicationSettings->value( SETTINGS_LOCK_FILE_NAME ).toString();
	logFileName = applicationSettings->value( SETTINGS_LOG_FILE_NAME ).toString();
	logDebugMessage = applicationSettings->value( SETTINGS_LOG_DEBUG_MESSAGE ).toBool();
	maxLogLines = applicationSettings->value( SETTINGS_MAX_LOG_LINES ).toInt();
	includePatternFileName = applicationSettings->value( SETTINGS_INCLUDE_PATTERN_FILE_NAME ).toString();
	excludePatternFileName = applicationSettings->value( SETTINGS_EXCLUDE_PATTERN_FILE_NAME ).toString();
	rsyncTimeout = applicationSettings->value( SETTINGS_RSYNC_TIMEOUT ).toInt();
	applicationSettings->endGroup();
	// [Server]
	applicationSettings->beginGroup( SETTINGS_GROUP_SERVER );
	defaultServerKey = applicationSettings->value( SETTINGS_SERVER_KEY ).toString();
	defaultServerName = applicationSettings->value( SETTINGS_HOST ).toString();
	backupFolderName = applicationSettings->value( SETTINGS_BACKUP_FOLDER_NAME ).toString();
	metaFolderName = applicationSettings->value( SETTINGS_META_FOLDER_NAME ).toString();
	backupRootFolder = applicationSettings->value( SETTINGS_BACKUP_ROOT_FOLDER ).toString();
	restoreRootFolder = applicationSettings->value( SETTINGS_RESTORE_ROOT_FOLDER ).toString();
	metadataFileName = applicationSettings->value( SETTINGS_METADATA_FILE_NAME ).toString();
	backupContentFileName = applicationSettings->value( SETTINGS_BACKUP_CONTENT_FILE_NAME ).toString();
	backupTimeFileName = applicationSettings->value( SETTINGS_BACKUP_TIME_FILE_NAME ).toString();
	serverQuotaScriptName = applicationSettings->value( SETTINGS_SERVER_QUOTA_SCRIPT_NAME ).toString();
	authorizedKeyFolderName = applicationSettings->value( SETTINGS_AUTHORIZED_KEY_FOLDER_NAME ).toString();
	authorizedKeyFileName = applicationSettings->value( SETTINGS_AUTHORIZED_KEY_FILE_NAME ).toString();
	quotaModificationUrl = applicationSettings->value( SETTINGS_QUOTA_MODIFICATION_URL ).toString();
	quotaModificationUrlUidParam = applicationSettings->value( SETTINGS_QUOTA_MODIFICATION_URL_UID_PARAM ).toString();
	applicationSettings->endGroup();

	// [Executables]
	applicationSettings->beginGroup( SETTINGS_GROUP_EXECUTABLES );
	thisApplication = StringUtils::encaps( EXECUTABLE_NAME, "", SETTINGS_EXECUTABLE_EXTENSION );
	rsync = StringUtils::encaps( applicationSettings->value( SETTINGS_RSYNC ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	plink = StringUtils::encaps( applicationSettings->value( SETTINGS_PLINK ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	ssh = StringUtils::encaps( applicationSettings->value( SETTINGS_SSH ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	getfacl = StringUtils::encaps( applicationSettings->value( SETTINGS_GETFACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	setfacl = StringUtils::encaps( applicationSettings->value( SETTINGS_SETFACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	setacl = StringUtils::encaps( applicationSettings->value( SETTINGS_SETACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION );
	applicationSettings->endGroup();

	// [Reseller]
	resellerAddress = resellerSettings->value( SETTINGS_GROUP_RESELLER + "/" + SETTINGS_RESELLER_ADDRESS, QObject::tr( "reseller address (missing)" ) ).toString();


	resellerSettings->beginGroup( SETTINGS_GROUP_SERVER );
	serverName = resellerSettings->value( SETTINGS_HOST, defaultServerName ).toString();
	serverKey = resellerSettings->value( SETTINGS_SERVER_KEY, serverKey ).toString();
	backupRootFolder = resellerSettings->value( SETTINGS_BACKUP_ROOT_FOLDER, backupRootFolder ).toString();
	restoreRootFolder = resellerSettings->value( SETTINGS_RESTORE_ROOT_FOLDER, restoreRootFolder ).toString();
	quotaModificationUrl = resellerSettings->value( SETTINGS_QUOTA_MODIFICATION_URL, quotaModificationUrl ).toString();
	quotaModificationUrlUidParam = resellerSettings->value( SETTINGS_QUOTA_MODIFICATION_URL_UID_PARAM, quotaModificationUrlUidParam ).toString();
	resellerSettings->endGroup();

	applicationName = resellerSettings->value( SETTINGS_GROUP_APPLICATION + "/" + SETTINGS_APPLICATION_FULL_NAME, applicationName ).toString();

	serverName = userSettings->value( SETTINGS_HOST, serverName ).toString();
	serverUserName = userSettings->value( SETTINGS_USERNAME ).toString();
	languageIndex = userSettings->value( SETTINGS_LANGUAGE ).toInt();

	backupPrefix = userSettings->value( SETTINGS_BACKUP_PREFIX ).toString();
	if ( backupPrefix == "" )
	{
		saveBackupPrefix( getLocalHostName() );
	}
	serverKey = userSettings->value( SETTINGS_SERVER_KEY, serverKey ).toString();
	if ( serverKey == "" )
	{
		serverKey = defaultServerKey;
	}
	privatePuttyKey = userSettings->value( SETTINGS_PRIVATE_PUTTY_KEY ).toString();
	privateOpenSshKey = userSettings->value( SETTINGS_PRIVATE_OPEN_SSH_KEY ).toString();
	deleteExtraneousItems = true; // userSettings->value( SETTINGS_DELETE_EXTRANEOUS_ITEMS ).toBool();
	windowSize = userSettings->value( SETTINGS_WINDOW_SIZE ).toSize();
	windowPosition = userSettings->value( SETTINGS_WINDOW_POSITION, QPoint( 200, 200 ) ).toPoint();

	// [AppData]
	appData->beginGroup( SETTINGS_GROUP_APPDATA );
	int size = appData->beginReadArray( SETTINGS_APPDATA_LASTBACKUPS );
	lastBackups.clear();
	for ( int i = 0; i < size; ++i )
	{
		appData->setArrayIndex( i );
		ConstUtils::StatusEnum status = (ConstUtils::StatusEnum)(appData->value( SETTINGS_APPDATA_LASTBACKUP_STATUS ).toInt());
		status = status==ConstUtils::STATUS_UNDEFINED ? ConstUtils::STATUS_ERROR : status;
		lastBackups.append( BackupTask( appData->value( SETTINGS_APPDATA_LASTBACKUP_DATE ).toDateTime(), status ) );
	}
	appData->endArray();
	scheduleRule = appData->value( SETTINGS_SCHEDULE_RULE ).value<ScheduledTask>();

	qRegisterMetaType< QPair<QString,bool> >("QPair<QString,bool>");
	size = appData->beginReadArray( SETTINGS_LAST_BACKUP_RULES );
	lastBackupRules.clear();
	for ( int i = 0; i < size; ++i )
	{
		appData->setArrayIndex( i );
		bool rule_modifier = appData->value( SETTINGS_LAST_BACKUP_RULES_MODIFIER ).toBool();
		QString rule_item = appData->value( SETTINGS_LAST_BACKUP_RULES_ITEM ).toString();
		lastBackupRules.insert( rule_item, rule_modifier );
	}
	appData->endArray();
	appData->endGroup();

	// [GUI]
	appData->beginGroup( SETTINGS_GROUP_GUI );
	nOfLastBackups = appData->value( SETTINGS_N_OF_SHOWN_LAST_BACKUPS, DEFAULT_NUM_OF_LAST_BACKUPS ).toInt();
	appData->endGroup();

	this->settingsChanged = false;
}

QDateTime Settings::getInstallDate()
{
	QFileInfo applicationFile( QCoreApplication::applicationFilePath() );
	return applicationFile.created();
}

bool Settings::isReinstalled()
{
	return installDate < getInstallDate();
}

bool Settings::isInevitableSettingsMissing()
{
	return this->backupPrefix == 0 || this->backupPrefix == "" || this->serverUserName == 0 || this->serverUserName == "";
}

void Settings::deleteSettings()
{
	foreach( QString key, userSettings->allKeys() )
	{
		userSettings->remove( key );
	}
	saveInstallDate( getInstallDate() );
	reloadSettings();
}

void Settings::keepSettings()
{
	saveInstallDate( getInstallDate() );
}

void Settings::saveInstallDate( const QDateTime& installDate, bool force_write )
{
	if ( installDate != this->installDate )
	{
		userSettings->setValue( SETTINGS_INSTALL_DATE, installDate );
		this->installDate = installDate;
	}
	if ( force_write && this->userSettings )
	{
		this->userSettings->sync();
	}
}

QString Settings::getLocalHostName()
{
	return QHostInfo::localHostName();
}

QString Settings::createPrivatePuttyKeyFile()
{
	QString filePath = this->getApplicationDataDir() + this->privatePuttyKeyFileName;
	if ( createKeyFile( this->privatePuttyKey, filePath ) )
	{
		return filePath;
	}
	return "";
}

QString Settings::createPrivateOpenSshKeyFile()
{
	QString filePath = this->getApplicationDataDir() + this->privateOpenSshKeyFileName;
	if ( createKeyFile( this->privateOpenSshKey, filePath ) )
	{
		return filePath;
	}
	return "";
}

bool Settings::createKeyFile( const QString& key, const QString& keyFilePath )
{
	if ( key == "" )
	{
		qDebug() << "Key has not been generated";
		return false;
	}

	QFile privateKeyFile( keyFilePath );
	if ( !privateKeyFile.exists() )
	{
		if ( !privateKeyFile.open( QIODevice::WriteOnly ) )
		{
			qCritical() << "Can not write to key file: " << keyFilePath;
			return false;
		}
		if ( !privateKeyFile.setPermissions( QFile::ReadOwner | QFile::WriteOwner ) )
		{
			qWarning() << "Permission for private key file can not be set";
		}
		QTextStream out( &privateKeyFile );
		out << key;
		privateKeyFile.close();
	}
	return true;
}

void Settings::deletePrivateKeyFiles()
{
	FileSystemUtils::removeFile( this->getApplicationDataDir() + this->privatePuttyKeyFileName );
	FileSystemUtils::removeFile( this->getApplicationDataDir() + this->privateOpenSshKeyFileName );
}

void Settings::setServerPassword( const QString& password )
{
	if ( this->server_password != password )
	{
		this->server_password = password;
	}
}

void Settings::setClientPassword( const QString& password )
{
	if ( this->client_password != password )
	{
		this->client_password = password;
	}
}

void Settings::setServerUserNameAndPassword( const QString& userName, const QString& password, const bool isUsernameEditable )
{
	this->setServerPassword( password );
	if ( isUsernameEditable )
	{
		this->saveServerUserName( userName );
	}
}

void Settings::setClientUserNameAndPassword( const QString& userName, const QString& password, const bool isUsernameEditable )
{
	this->setClientPassword( password );
}

void Settings::savePrivatePuttyKey( const QString& privatePuttyKey )
{
	if ( this->privatePuttyKey != privatePuttyKey )
	{
		this->privatePuttyKey = privatePuttyKey;
		userSettings->setValue( SETTINGS_PRIVATE_PUTTY_KEY, privatePuttyKey );
	}
}

void Settings::savePrivateOpenSshKey( const QString& privateOpenSshKey )
{
	if ( this->privateOpenSshKey != privateOpenSshKey )
	{
		this->privateOpenSshKey = privateOpenSshKey;
		userSettings->setValue( SETTINGS_PRIVATE_OPEN_SSH_KEY, privateOpenSshKey );
	}
}

void Settings::saveServerKey( const QString& serverKey )
{
	if ( this->serverKey != serverKey )
	{
		this->serverKey = serverKey;
		userSettings->setValue( SETTINGS_SERVER_KEY, serverKey );
	}
}

void Settings::saveServerUserName( const QString &userName )
{
	if ( this->serverUserName != userName )
	{
		this->serverUserName = userName;
		userSettings->setValue( SETTINGS_USERNAME, userName );
		// delete the private key if the username has changed
		savePrivatePuttyKey( "" );
		savePrivateOpenSshKey( "" );
	}
}

void Settings::saveLanguageIndex( const int& languageIndex )
{
	if ( this->languageIndex != languageIndex )
	{
		this->languageIndex = languageIndex;
		userSettings->setValue( SETTINGS_LANGUAGE, languageIndex );
	}
}

void Settings::saveServerName( const QString& serverName )
{
	if ( this->serverName != serverName )
	{
		userSettings->setValue( SETTINGS_HOST, serverName );
		this->serverName = serverName;
	}
}


QString Settings::getMetadataFileName()
{
	if ( IS_MAC )
	{
		return metadataFileName + "_mac";
	}
	if ( IS_WINDOWS )
	{
		return metadataFileName + "_win";
	}
	return metadataFileName + "_unix";
}

QString Settings::getTempMetadataFileName()
{
	return getMetadataFileName() + "_tmp";
}

void Settings::saveBackupPrefix( const QString& backupPrefix )
{
	if ( this->backupPrefix != backupPrefix )
	{
		this->backupPrefix = backupPrefix;
		userSettings->setValue( SETTINGS_BACKUP_PREFIX, backupPrefix );
	}
}

const char* Settings::getEOLCharacter()
{
	if ( IS_WINDOWS )
	{
		return "\r\n";
	}
	return "\n";
}

void Settings::saveWindowSize( QSize size )
{
	if ( this->windowSize != size )
	{
		this->windowSize = size;
		userSettings->setValue( SETTINGS_WINDOW_SIZE, size );
	}
}

void Settings::saveWindowPosition( QPoint position )
{
	if ( this->windowPosition != position )
	{
		this->windowPosition = position;
		userSettings->setValue( SETTINGS_WINDOW_POSITION, position );
	}
}

void Settings::saveNOfLastBackups( int nOfLastBackups )
{
	if ( this->nOfLastBackups != nOfLastBackups )
	{
		nOfLastBackups = std::min( 10, std::max( 0, nOfLastBackups ) );
		this->nOfLastBackups = nOfLastBackups;
		appData->setValue( SETTINGS_GROUP_GUI + "/" + SETTINGS_N_OF_SHOWN_LAST_BACKUPS, nOfLastBackups );
	}
}

/**
 * adds a new Task to the list of last Backup Tasks
 * overwrites the lastBackupTask if its time is equal to the passed BackupTask's backupTime or if passed if it is equal to the passed QDateTime originalStartDateTime (f.ex. to be able to reset time to end of backup)
 */
void Settings::addLastBackup( const BackupTask& lastBackup )
{
	if ( this->lastBackups.size() == 0 || !this->lastBackups.at( 0 ).equals( lastBackup ) ) {
		// identical values are not repeated
		this->lastBackups.prepend( lastBackup );
	}
	this->saveLastBackups(this->lastBackups);
}

void Settings::replaceLastBackup( const BackupTask& newBackupInfo ) {
	if ( this->lastBackups.size() != 0 ) {
		this->lastBackups.takeFirst();
		this->lastBackups.prepend( newBackupInfo );
	}
	this->saveLastBackups(this->lastBackups);
}

void Settings::saveLastBackups( const QList<BackupTask>& lastBackups )
{
	qDebug() << "Settings::saveLastBackups(...)";
	qDebug() << "saving lastBackup-settings into file:" << this->appData->fileName();
	appData->beginGroup( SETTINGS_GROUP_APPDATA );
	appData->beginWriteArray( SETTINGS_APPDATA_LASTBACKUPS );
	for ( int i = 0; i < std::min( this->lastBackups.size(), Settings::MAX_SAVED_LAST_BACKUPS ); ++i )
	{
		appData->setArrayIndex( i );
		appData->setValue( SETTINGS_APPDATA_LASTBACKUP_DATE, this->lastBackups.at( i ).getDateTime() );
		appData->setValue( SETTINGS_APPDATA_LASTBACKUP_STATUS, ( int )this->lastBackups.at( i ).getStatus() );
	}
	appData->endArray();
	appData->endGroup();
	appData->sync(); // simple fix for the issue, that lastBackups were never persisted in -schedule-runs
}

void Settings::saveScheduleRule(const ScheduledTask& scheduleRule)
{
	if ( !this->scheduleRule.equals(scheduleRule) )
	{
		appData->beginGroup( SETTINGS_GROUP_APPDATA );
		this->scheduleRule = scheduleRule;

		QVariant var;
		var.setValue(scheduleRule);
		appData->setValue( SETTINGS_SCHEDULE_RULE, var );
		appData->endGroup();
	}
}

void Settings::saveBackupSelectionRules( const BackupSelectionHash& backupSelectionRules )
{
	qDebug() << "Settings::saveBackupSelectionRules(" << backupSelectionRules << ")";
	this->lastBackupRules = backupSelectionRules;
	appData->beginGroup( SETTINGS_GROUP_APPDATA );
	appData->beginWriteArray( SETTINGS_LAST_BACKUP_RULES );
	QList<QString> items = this->lastBackupRules.keys();
	int i = 0;
	foreach ( QString item, items )
	{
		appData->setArrayIndex( i );
		qDebug() << "Settings::saveBackupSelectionRules(...)" << "writing:" << this->lastBackupRules[item] << item;
		appData->setValue( SETTINGS_LAST_BACKUP_RULES_MODIFIER, this->lastBackupRules[item] );
		appData->setValue( SETTINGS_LAST_BACKUP_RULES_ITEM, item );
		i++;
	}
	appData->endArray();
	appData->endGroup();
}
