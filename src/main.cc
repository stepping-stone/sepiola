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

#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFileInfo>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#ifdef Q_OS_UNIX
#include <pwd.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "cli/cli_manager.hh"
#include "gui/main_window.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "utils/log_file_utils.hh"

namespace
{
static const QString CONFIG_FILE_NAME = "config";
static const QString VERSION_ARGUMENT = "-version";
static const char* DEPENDENCY_MISSING = "Dependent files are missing. Please reinstall the application.\n";
static const char* SINGLE_APPLICATION_ERROR =
		"The application is already running. Only one instance of this application can be run concurrently.\n";
}

bool initSettings(QCoreApplication* app)
{
	QString applicationDirPath = app->applicationDirPath();
	QFileInfo configFile(applicationDirPath, CONFIG_FILE_NAME);
	if ( !configFile.exists() )
	{
		qDebug() << "config file: " + applicationDirPath + "/" + CONFIG_FILE_NAME + " not found.";
		return false;
	}
	Settings::getInstance()->loadSettings(configFile);
	return true;
}

void messageHandler(QtMsgType type, const char *msg)
{
	if (Settings::getInstance()->isLogDebugMessageEnabled() )
	{
		QString message;
		message.append(msg);
		message.append(Settings::getInstance()->getEOLCharacter() );
		switch (type)
		{
		case QtDebugMsg:
		case QtWarningMsg:
			fprintf( stdout, message.toLocal8Bit());
			fflush( stdout);
			break;
		case QtCriticalMsg:
		case QtFatalMsg:
			fprintf( stderr, message.toLocal8Bit());
			fflush( stderr);
			break;
		}
	}
}

bool assertCliDependencies()
{
	Settings* settings = Settings::getInstance();
	QFileInfo plinkFile(settings->getPlinkName() );
	QFileInfo rsyncFile(settings->getRsyncName() );
	bool sshClient = true;
	if (settings->useOpenSshInsteadOfPlinkForRsync() )
	{
		QFileInfo sshFile(settings->getSshName() );
		sshClient = sshFile.exists();
	}
	if (Settings::IS_WINDOWS)
	{
		QFileInfo setaclFile(settings->getSetAclName() );
		return plinkFile.exists() && rsyncFile.exists() && setaclFile.exists() && sshClient;
	}
	else if (Settings::IS_MAC)
	{
		return plinkFile.exists() && rsyncFile.exists() && sshClient;
	}
	QFileInfo getfaclFile(settings->getGetfaclName() );
	QFileInfo setfaclFile(settings->getSetfaclName() );
	return getfaclFile.exists() && plinkFile.exists() && rsyncFile.exists() && setfaclFile.exists() && sshClient;
}

bool assertGuiDependencies()
{
	QTranslator translator;
	Settings* settings = Settings::getInstance();
	QStringList supportedLanguages = settings->getSupportedLanguages();
	for (int i=1; i<supportedLanguages.size(); i++)
	{
		// skip the default langauge, because there is no translation file for it
		if (!translator.load(supportedLanguages.at(i), settings->getApplicationBinDir() ) )
		{
			return false;
		}
	}
	return assertCliDependencies();
}

bool isProcessRunning(const int& processId)
{
#ifdef Q_OS_UNIX
	return (processId != 0 ) && ( !kill(processId, 0) );
#else
	return ( processId != 0) && ( GetProcessVersion( processId ) != 0 );
#endif
}

bool assertSingleApplication()
{
	Settings* settings = Settings::getInstance();
	QFile lockFile(settings->getApplicationDataDir() + settings->getLockFileName() );
	if ( !lockFile.open(QIODevice::ReadWrite) )
	{
		fprintf( stderr, "Can not read lock file\n");
		return false;
	}
	QTextStream in( &lockFile);
	int existingProcessId;
	in >> existingProcessId;
	lockFile.close();
	int currentProcessId = getpid();

	if (existingProcessId != currentProcessId)
	{
		// find out if the existingProcessId is still running
		if (isProcessRunning(existingProcessId) )
		{
			return false;
		}

		if ( !lockFile.open(QIODevice::WriteOnly | QIODevice::Truncate) )
		{
			fprintf( stderr, "Can not write to lock file\n");
			return false;
		}
		QTextStream out( &lockFile);
		out << currentProcessId;
		lockFile.close();
		return true;
	}
	return false;
}

void setHomeDir()
{
#ifdef Q_OS_UNIX
	uid_t userId = geteuid(); // get the real user ID of the current process
	setenv("HOME", getpwuid( userId )->pw_dir, 1); // set the HOME environment variable to the real home directory
#endif
}

void createConsole()
{
#ifdef Q_OS_WIN
	int hConHandle;
	long lStdHandle;
	FILE *fp;

	AllocConsole();

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	// ios::sync_with_stdio();
#endif
}

int main(int argc, char *argv[])
{
	setHomeDir();

	if (CliManager::isNoopApplication(argc, argv) )
	{
		return 0;
	}

	if (CliManager::isHelpApplication(argc, argv) )
	{
		QCoreApplication app(argc, argv);
		if ( !initSettings( &app) || !assertCliDependencies() )
		{
			fprintf( stderr, DEPENDENCY_MISSING);
			return -1;
		}
		CliManager::printUsage(argc, argv);
		return 0;
	}

	if (TestManager::isTestApplication(argc, argv) )
	{
		// run the test(s) and exit
		QCoreApplication app(argc, argv);
		if ( !initSettings( &app) || !assertCliDependencies() )
		{
			fprintf( stderr, DEPENDENCY_MISSING);
			return -1;
		}
		LogFileUtils::getInstance()->open();
		qInstallMsgHandler(messageHandler);
		TestManager::run(argc, argv);
		app.exit();
		LogFileUtils::getInstance()->close();
		return 0;
	}

	if (CliManager::isCliApplication(argc, argv) )
	{
		// run the command line version
		QCoreApplication app(argc, argv);
		if ( !initSettings( &app) || !assertCliDependencies() )
		{
			fprintf( stderr, DEPENDENCY_MISSING);
			return -1;
		}
		if ( !assertSingleApplication() )
		{
			fprintf( stderr, SINGLE_APPLICATION_ERROR);
			return -1;
		}
		LogFileUtils::getInstance()->open();
		qInstallMsgHandler(messageHandler);
		CliManager cliManager;
		cliManager.runCli(argc, argv);
		app.exit();
		LogFileUtils::getInstance()->close();
		return 0;
	}
	if (CliManager::isScheduleApplication(argc, argv) )
	{
		// run the scheduled job
		QCoreApplication app(argc, argv);
		if ( !initSettings( &app) || !assertCliDependencies() )
		{
			fprintf( stderr, DEPENDENCY_MISSING);
			return -1;
		}
		if ( !assertSingleApplication() )
		{
			fprintf( stderr, SINGLE_APPLICATION_ERROR);
			return -1;
		}
		LogFileUtils::getInstance()->open();
		qInstallMsgHandler(messageHandler);
		CliManager::runSchedule();
		app.exit();
		LogFileUtils::getInstance()->close();
		return 0;
	}

	// run with gui
	QApplication app(argc, argv);
	if ( !initSettings( &app) || !assertGuiDependencies() )
	{
		QMessageBox::critical( 0, "Dependency missing", DEPENDENCY_MISSING);
		return -1;
	}
	if ( !assertSingleApplication() )
	{
		QMessageBox::critical( 0, "Application is already running", SINGLE_APPLICATION_ERROR);
		return -1;
	}
	//createConsole();
	LogFileUtils::getInstance()->open();
	qInstallMsgHandler(messageHandler);
	MainModel model;

	QTranslator translator;
	Settings* settings = Settings::getInstance();
	translator.load(settings->getSupportedLanguages().at(settings->getLanguageIndex() ), settings->getApplicationBinDir() );
	app.installTranslator( &translator);

	MainWindow mainWindow( &model);
	QObject::connect( &mainWindow, SIGNAL( writeLog( const QString& ) ), LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ));

	int result = 0;
	if (CliManager::isUpdateOnlyApplication(argc, argv) )
	{
		LogFileUtils::getInstance()->writeLog(QObject::tr("Update started") );
		// set update date
		Settings* settings = Settings::getInstance(); 
		settings->saveInstallDate( settings->getInstallDate(), true );
	}
	else
	{
		mainWindow.show();
		LogFileUtils::getInstance()->writeLog(QObject::tr("Application started") );
		result = app.exec();
		LogFileUtils::getInstance()->writeLog(QObject::tr("Application closed") );
	}
	LogFileUtils::getInstance()->close();
	QObject::disconnect( &mainWindow, SIGNAL( writeLog( const QString& ) ), LogFileUtils::getInstance(), SLOT( writeLog( const QString& ) ));
	return result;
}
