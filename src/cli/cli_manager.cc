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

#include <iostream>
#include <stdio.h>

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>

#ifdef Q_OS_UNIX
#include <termios.h>
#endif

#include "cli/cli_manager.hh"
#include "exception/login_exception.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "utils/file_system_utils.hh"
#include "utils/log_file_utils.hh"

const QString CliManager::NOOP_ARGUMENT = "-noop";
const QString CliManager::HELP_ARGUMENT = "--help";
const QString CliManager::CLI_ARGUMENT = "-cli";
const QString CliManager::SCHEDULE_ARGUMENT = "-schedule";
const QString CliManager::UPDATE_ONLY_ARGUMENT = "-update";
const QString CliManager::INCLUDE_PATTERN_FILE_ARGUMENT = "-includeFile=";
const QString CliManager::EXCLUDE_PATTERN_FILE_ARGUMENT = "-excludeFile=";
const QString CliManager::BACKUP_LIST_FILE_ARGUMENT = "-backupFile=";
const QString CliManager::DELETE_ARGUMENT = "-delete";

CliManager::CliManager()
{
}

CliManager::~CliManager()
{
}

bool CliManager::hasArgument( int argc, char* argv[], const QString& argumentName )
{
	for ( int i=1; i<argc; i++ )
	{
		if ( argumentName.compare( argv[i] ) == 0 )
		{
			return true;
		}
	}
	return false;
}

bool CliManager::isNoopApplication( int argc, char* argv[] )
{
	return hasArgument( argc, argv, NOOP_ARGUMENT );
}

bool CliManager::isHelpApplication( int argc, char* argv[] )
{
	return hasArgument( argc, argv, HELP_ARGUMENT );
}

void CliManager::printUsage( int argc, char* argv[] )
{
	QString usage;
	usage.append( Settings::getInstance()->getThisApplicationFullPathExecutable() + " Version " + Settings::VERSION + "\n" );
	usage.append( "Copyright (C) 2007 by stepping stone GmbH\n" );
	usage.append( "Usage: " );
	usage.append( argv[0] );
	usage.append( " [Options]\n\n" );
	usage.append( "Options\n" );
	usage.append( " --help\t\t\tshow this help\n" );
	usage.append( " " + SCHEDULE_ARGUMENT + "\t\trun the scheduled job now\n" );
	usage.append( " " + CLI_ARGUMENT + "\t\t\trun the application without a graphical user interface\n" );
	usage.append( "The following options only apply to " + CLI_ARGUMENT + " argument:\n" );
	usage.append( " " + BACKUP_LIST_FILE_ARGUMENT + "\t\tfile containing a list items to backup\n" );
	usage.append( " [" + DELETE_ARGUMENT + "]\t\tdelete extraneous files on the server\n" );
	std::cout << usage.toUtf8().data();
}

bool CliManager::isCliApplication( int argc, char* argv[] )
{
	return hasArgument( argc, argv, CLI_ARGUMENT );
}

bool CliManager::isScheduleApplication( int argc, char* argv[] )
{
	return hasArgument( argc, argv, SCHEDULE_ARGUMENT );
}

bool CliManager::isUpdateOnlyApplication( int argc, char* argv[] )
{
	return hasArgument( argc, argv, UPDATE_ONLY_ARGUMENT );
}

void CliManager::runSchedule()
{
	Settings* settings = Settings::getInstance();
	int delay = settings->getSchedulerDelay();
	bool deleteExtraneousItems = settings->getDeleteExtraneousItems();

	if ( delay > 0 )
	{
		qDebug() << "Starting schedule job in " << delay << " minute(s)";
		QWaitCondition waitCondition;
		QMutex mutex;
		mutex.lock();
		waitCondition.wait( &mutex, delay * 1000 * 60 );
		qDebug() << "Starting schedule job now";
	}

	QStringList backupList = settings->getBackupList();
	MainModel mainModel;

	QObject::connect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
							LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
							LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );

	mainModel.backup( backupList, settings->getIncludePatternList(), settings->getExcludePatternList(), deleteExtraneousItems, true );

	QObject::disconnect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
								 LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
								 LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
}

void CliManager::runCli( int argc, char* argv[] )
{
	if ( !assertArguments( argc, argv ) )
	{
		printUsage( argc, argv );
		return;
	}

	QStringList includePatternList;
	QStringList excludePatternList;
	QStringList backupList;
	bool deleteExtraneousItems = hasArgument( argc, argv, DELETE_ARGUMENT );


	QString backupListFileName = getArgumentsValue( argc, argv, BACKUP_LIST_FILE_ARGUMENT );
	backupList = FileSystemUtils::readLinesFromFile( backupListFileName, "UTF-8" );

	if ( hasArgumentValue( argc, argv, INCLUDE_PATTERN_FILE_ARGUMENT ) )
	{
		QString includeFileName = getArgumentsValue( argc, argv, INCLUDE_PATTERN_FILE_ARGUMENT );
		includePatternList = FileSystemUtils::readLinesFromFile( includeFileName, "UTF-8" );
	}
	if ( hasArgumentValue( argc, argv, EXCLUDE_PATTERN_FILE_ARGUMENT ) )
	{
		QString excludeFileName = getArgumentsValue( argc, argv, EXCLUDE_PATTERN_FILE_ARGUMENT );
		excludePatternList = FileSystemUtils::readLinesFromFile( excludeFileName, "UTF-8" );
	}
	MainModel mainModel;
	QObject::connect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
						this, SLOT( printInfoMessage( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
						this, SLOT( printErrorMessage( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( showInformationMessageBox( const QString& ) ),
						this, SLOT( printInfoMessage( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( showCriticalMessageBox( const QString& ) ),
						this, SLOT( printErrorMessage( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( askForServerPassword( const QString&, bool, int*, const QString& ) ),
						this, SLOT( askForPassword() ) );
	QObject::connect( &mainModel, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ),
						this, SLOT( askForPassword() ) );

	// log file signals/slots
	QObject::connect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
							LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
	QObject::connect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
							LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );

	mainModel.backup( backupList, includePatternList, excludePatternList, deleteExtraneousItems, true );

	QObject::disconnect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
							 this, SLOT( printInfoMessage( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
							 this, SLOT( printErrorMessage( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( showInformationMessageBox( const QString& ) ),
							 this, SLOT( printInfoMessage( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( showCriticalMessageBox( const QString& ) ),
							 this, SLOT( printErrorMessage( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( askForServerPassword( const QString&, bool, int*, const QString& ) ),
							 this, SLOT( askForPassword() ) );
	QObject::disconnect( &mainModel, SIGNAL( askForClientPassword( const QString&, bool, int*, const QString& ) ),
							 this, SLOT( askForPassword() ) );

	// log file signals/slots
	QObject::disconnect( &mainModel, SIGNAL( appendInfoMessage( const QString& ) ),
								 LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
	QObject::disconnect( &mainModel, SIGNAL( appendErrorMessage( const QString& ) ),
								 LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ) );
}

void CliManager::printInfoMessage( const QString& message )
{
	std::cout << message.toUtf8().data() << "\n";
}

void CliManager::printErrorMessage( const QString& message )
{
	std::cerr << message.toUtf8().data() << "\n";
}

void CliManager::askForPassword()
{
	QTextStream cin( stdin, QIODevice::ReadOnly );
	QTextStream cout( stdout, QIODevice::WriteOnly );

	cout << "Enter username: " << flush;
	QString username;
	cin >> username;

	cout << "Enter password: " << flush;
	QString password = readPassword();

	Settings* settings = Settings::getInstance();
	settings->saveServerUserName( username );
	settings->setServerPassword( password );
}

QString CliManager::readPassword()
{
	#ifdef Q_OS_UNIX
		struct termios oldTermios, newTermios;

		// Turn echoing off and fail if we can't.
		if ( tcgetattr ( 0, &oldTermios ) != 0)
		{
			qWarning() << "Can not turn off password echoing";
		}
		newTermios = oldTermios;
		newTermios.c_lflag &= ~ECHO;
		if ( tcsetattr( 0, TCSAFLUSH, &newTermios ) != 0 )
		{
			qWarning() << "Can not turn off password echoing";
		}
	#endif

	// Read the password.
	QString password;
	QTextStream cin( stdin, QIODevice::ReadOnly );
	cin >> password;

	#ifdef Q_OS_UNIX
		// Restore terminal.
		(void) tcsetattr( 0, TCSAFLUSH, &oldTermios );
	#endif

	return password;
}

bool CliManager::assertArguments( int argc, char* argv[] )
{
	return hasArgumentValue( argc, argv, BACKUP_LIST_FILE_ARGUMENT );
}

bool CliManager::hasArgumentValue( int argc, char* argv[], const QString& argumentName  )
{
	bool result = false;
	for ( int i=1; i<argc; i++ )
	{
		QString argument = argv[i];
		if ( argument.startsWith( argumentName ) && argument.size() > argumentName.size() )
		{
			result = true;
			break;
		}
	}
	return result;
}

QString CliManager::getArgumentsValue( int argc, char* argv[], const QString& argumentName )
{
	for ( int i=1; i<argc; i++ )
	{
		QString argument = argv[i];
		if ( argument.startsWith( argumentName ) )
		{
			return argument.mid( argumentName.size() , argument.size() - argumentName.size() );
		}
	}
	QString message = "No value for argument " + argumentName + " found";
	std::cerr << message.toUtf8().data();
	return "";
}
