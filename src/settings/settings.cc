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

#include "settings/settings.hh"
#include "utils/file_system_utils.hh"
#include "utils/string_utils.hh"
#include "utils/log_file_utils.hh"

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

const QString Settings::EXECUTABLE_NAME = QString(SSBACKUP_EXECUTABLE_NAME);
const QString Settings::VERSION = QString::number(CPACK_PACKAGE_VERSION_MAJOR) + "." +
                                  QString::number(CPACK_PACKAGE_VERSION_MINOR) + "." +
                                  QString::number(CPACK_PACKAGE_VERSION_PATCH);
const bool Settings::IS_RESELLER = (CPACK_IS_RESELLER==1 ? true : false);

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
const QString Settings::SETTINGS_AUTHORIZED_KEY_FOLDER_NAME = "AuthorizedKeyFolderName";
const QString Settings::SETTINGS_AUTHORIZED_KEY_FILE_NAME = "AuthorizedKeyFileName";

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
const QString Settings::SETTINGS_BACKUP_LIST = "BackupList";
const QString Settings::SETTINGS_SERVER_KEY = "ServerKey";
const QString Settings::SETTINGS_PRIVATE_PUTTY_KEY = "PrivateKey";
const QString Settings::SETTINGS_PRIVATE_OPEN_SSH_KEY = "PrivateOpenSshKey";
const QString Settings::SETTINGS_SCHEDULER_DELAY = "SchedulerDelay";
const QString Settings::SETTINGS_DELETE_EXTRANEOUS_ITEMS = "DeleteExtraneousItems";
const QString Settings::SETTINGS_WINDOW_POSITION = "WindowPosition";
const QString Settings::SETTINGS_WINDOW_SIZE = "WindowSize";

// [Reseller]
const QString Settings::SETTINGS_GROUP_RESELLER = "Reseller";
const QString Settings::SETTINGS_RESELLER_ADDRESS = "ResellerAddress";

Settings* Settings::instance = 0;

Settings::Settings()
{
	//TODO: load language name translations after setting the current langauge
	supportedLanguages << QObject::tr( "English" ); // default language has to be at the top (position zero)
	supportedLanguages << QObject::tr( "German" );
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

void Settings::loadSettings( const QFileInfo& configFile, const QString& resellerAffix )
{
	qDebug() << "Settings::loadSettings: Loading settings from file <" << configFile.filePath() << ">";
	QDir homeDir = QDir::home();
	QString applicationDataDirName = "." + EXECUTABLE_NAME;
	QString resellerSettingsFileName = configFile.absoluteFilePath() + resellerAffix;
	if ( !homeDir.exists( applicationDataDirName ) )
	{
		homeDir.mkdir( applicationDataDirName );
	}
	applicationDataDir = homeDir.absolutePath() + QDir::separator() + applicationDataDirName + QDir::separator();
	applicationBinDir = configFile.absolutePath() + QDir::separator();
	applicationSettings = new QSettings( configFile.absoluteFilePath(), QSettings::IniFormat );
	resellerSettings =    new QSettings( resellerSettingsFileName, QSettings::IniFormat );
	userSettings =        new QSettings( applicationDataDir + configFile.fileName(), QSettings::IniFormat );
	
	if (QFile(resellerSettingsFileName).exists()) {
		qDebug() << "Loading reseller-settings from file <" + resellerSettings->fileName() + ">";
	} else {
		qDebug() << "reseller-config-file not found <" + resellerSettingsFileName + ">";
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
	authorizedKeyFolderName = applicationSettings->value( SETTINGS_AUTHORIZED_KEY_FOLDER_NAME ).toString();
	authorizedKeyFileName = applicationSettings->value( SETTINGS_AUTHORIZED_KEY_FILE_NAME ).toString();
	applicationSettings->endGroup();

	// [Executables]
	applicationSettings->beginGroup( SETTINGS_GROUP_EXECUTABLES );
	thisApplication = StringUtils::encaps(EXECUTABLE_NAME, "", SETTINGS_EXECUTABLE_EXTENSION);
	rsync = StringUtils::encaps(applicationSettings->value( SETTINGS_RSYNC ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	plink = StringUtils::encaps(applicationSettings->value( SETTINGS_PLINK ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	ssh = StringUtils::encaps(applicationSettings->value( SETTINGS_SSH ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	getfacl = StringUtils::encaps(applicationSettings->value( SETTINGS_GETFACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	setfacl = StringUtils::encaps(applicationSettings->value( SETTINGS_SETFACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	setacl = StringUtils::encaps(applicationSettings->value( SETTINGS_SETACL ).toString(), "", SETTINGS_EXECUTABLE_EXTENSION);
	applicationSettings->endGroup();

	resellerAddress = resellerSettings->value( SETTINGS_GROUP_RESELLER + "/" + SETTINGS_RESELLER_ADDRESS, QObject::tr("reseller address (missing)") ).toString();

	
	resellerSettings->beginGroup( SETTINGS_GROUP_SERVER );
	serverName = resellerSettings->value( SETTINGS_HOST, defaultServerName ).toString();
	serverKey = resellerSettings->value( SETTINGS_SERVER_KEY, serverKey ).toString();
	backupRootFolder=resellerSettings->value( SETTINGS_BACKUP_ROOT_FOLDER, backupRootFolder ).toString();
	restoreRootFolder = resellerSettings->value( SETTINGS_RESTORE_ROOT_FOLDER, restoreRootFolder ).toString();
	resellerSettings->endGroup();

	applicationName = resellerSettings->value(SETTINGS_GROUP_APPLICATION + "/" + SETTINGS_APPLICATION_FULL_NAME, applicationName).toString();
	

	
	serverName = userSettings->value( SETTINGS_HOST, serverName ).toString();
	serverUserName = userSettings->value( SETTINGS_USERNAME ).toString();
	languageIndex = userSettings->value( SETTINGS_LANGUAGE ).toInt();

	backupPrefix = userSettings->value( SETTINGS_BACKUP_PREFIX ).toString();
	if( backupPrefix == "" )
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
	backupList = userSettings->value( SETTINGS_BACKUP_LIST ).toStringList();
	schedulerDelay = userSettings->value( SETTINGS_SCHEDULER_DELAY ).toInt();
	deleteExtraneousItems = userSettings->value( SETTINGS_DELETE_EXTRANEOUS_ITEMS ).toBool();

	windowSize = userSettings->value( SETTINGS_WINDOW_SIZE ).toSize();
	windowPosition = userSettings->value( SETTINGS_WINDOW_POSITION, QPoint( 200, 200 ) ).toPoint();

	this->settingsChanged = false;
}

void Settings::saveDeleteExtraneousItems( const bool& deleteExtraneousItems )
{
	if ( deleteExtraneousItems != this->deleteExtraneousItems )
	{
		userSettings->setValue( SETTINGS_DELETE_EXTRANEOUS_ITEMS, deleteExtraneousItems );
		this->deleteExtraneousItems = deleteExtraneousItems;
	}
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
	if (force_write && this->userSettings) {
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
		if (!privateKeyFile.setPermissions( QFile::ReadOwner | QFile::WriteOwner ) )
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

void Settings::saveBackupItemList( const QStringList& backupList )
{
	if( this->backupList != backupList )
	{
		this->backupList = backupList;
		userSettings->setValue( SETTINGS_BACKUP_LIST, backupList );
	}
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

void Settings::saveSchedulerDelay( const int& minutes )
{
	if ( this->schedulerDelay != minutes )
	{
		this->schedulerDelay = minutes;
		userSettings->setValue( SETTINGS_SCHEDULER_DELAY, minutes );
	}
}

void Settings::savePrivatePuttyKey ( const QString& privatePuttyKey )
{
	if ( this->privatePuttyKey != privatePuttyKey )
	{
		this->privatePuttyKey = privatePuttyKey;
		userSettings->setValue( SETTINGS_PRIVATE_PUTTY_KEY, privatePuttyKey );
	}
}

void Settings::savePrivateOpenSshKey ( const QString& privateOpenSshKey )
{
	if ( this->privateOpenSshKey != privateOpenSshKey )
	{
		this->privateOpenSshKey = privateOpenSshKey;
		userSettings->setValue( SETTINGS_PRIVATE_OPEN_SSH_KEY, privateOpenSshKey );
	}
}

void Settings::saveServerKey ( const QString& serverKey )
{
	if ( this->serverKey != serverKey )
	{
		this->serverKey = serverKey;
		userSettings->setValue( SETTINGS_SERVER_KEY, serverKey );
	}
}

void Settings::saveServerUserName ( const QString &userName )
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

QStringList Settings::getIncludePatternList()
{
	return FileSystemUtils::readLinesFromFile( applicationDataDir + includePatternFileName, "UTF-8" );
}

QStringList Settings::getExcludePatternList()
{
	return FileSystemUtils::readLinesFromFile( applicationDataDir + excludePatternFileName, "UTF-8" );
}

void Settings::saveIncludePatternList( const QStringList& includePatternList )
{
	FileSystemUtils::writeLinesToFile( applicationDataDir + includePatternFileName, includePatternList, "UTF-8" );
}

void Settings::saveExcludePatternList( const QStringList& excludePatternList )
{
	FileSystemUtils::writeLinesToFile( applicationDataDir + excludePatternFileName, excludePatternList, "UTF-8" );
}

void Settings::saveWindowSize( QSize size )
{
	if( this->windowSize != size )
	{
		this->windowSize = size;
		userSettings->setValue( SETTINGS_WINDOW_SIZE, size );
	}
}

void Settings::saveWindowPosition( QPoint position )
{
	if( this->windowPosition != position )
	{
		this->windowPosition = position;
		userSettings->setValue( SETTINGS_WINDOW_POSITION, position );
	}
}
