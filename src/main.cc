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

#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#ifdef Q_OS_UNIX
#include <pwd.h>
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#ifdef CMAKE_BUILD_TYPE_DEPLOY
	#include <QtPlugin>
	Q_IMPORT_PLUGIN(qsvg)
#endif

#include "cli/cli_manager.hh"
#include "gui/main_window.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "settings/platform.hh"
#include "test/test_manager.hh"
#include "utils/single_application_guard.hh"
#include "utils/log_file_utils.hh"

#include "config.h"

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
#ifdef PORTABLE_INSTALLATION
	QString applicationDirPath = app->applicationDirPath();
#else
    QString applicationDirPath = APPLICATION_SHARE_DIR;
#endif
	QFileInfo configFile(applicationDirPath, CONFIG_FILE_NAME);
	if ( !configFile.exists() )
	{
		qDebug() << "config file: " + configFile.absoluteFilePath() + " not found.";
		return false;
	}
	Settings::getInstance()->loadSettings(configFile);
	return true;
}

void messageHandler(QtMsgType type, const char *msg)
{
	if (Settings::getInstance()->isLogDebugMessageEnabled() )
	{
		switch (type)
		{
		case QtDebugMsg:
		case QtWarningMsg:
			fprintf(stdout, "%s%s", msg, Platform::EOL_CHARACTER);
			fflush(stdout);
			break;
		case QtCriticalMsg:
		case QtFatalMsg:
			fprintf(stderr, "%s%s", msg, Platform::EOL_CHARACTER);
			fflush(stderr);
			break;
		}
	}
}

bool assertCliDependencies()
{
	Settings* settings = Settings::getInstance();
	QFileInfo plinkFile(settings->getPlinkName() );
	QFileInfo rsyncFile(settings->getRsyncName() );

    // plink and rsync are required on all platforms
    if (!plinkFile.exists())
    {
        qDebug() << "plink" << settings->getPlinkName() << "could not be found";
        return false;
    }
    if (!rsyncFile.exists()) {
        qDebug() << "rsync" << settings->getRsyncName() << "could not be found";
        return false;
    }

    // return if the user chose to use the ssh client instead
    // of plink and the client could not befound
    QFileInfo sshFile(settings->getSshName() );
    if (!sshFile.exists())
    {
        qDebug() << "ssh" << settings->getSshName() << "could not be found";
        return false;
    }

#ifdef Q_OS_WIN32
	QFileInfo setaclFile(settings->getSetAclName() );
	return setaclFile.exists();
#endif

#ifdef Q_OS_UNIX
	QFileInfo getfaclFile(settings->getGetfaclName() );
	QFileInfo setfaclFile(settings->getSetfaclName() );
	return getfaclFile.exists() && setfaclFile.exists();
#endif

    return true;
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
    QString userName = settings->getClientUserName();

    //The userName is neede to create a shared memory segment for each user.
    //This allows each user to run one instance of the sepiola application.
    static SingleApplicationGuard singleApp( userName );
    if (singleApp.tryToCreateSharedMemory())
        return true;

    return false;
}

void setHomeDir()
{
#ifdef Q_OS_UNIX
	uid_t userId = geteuid(); // get the real user ID of the current process
	setenv("HOME", getpwuid( userId )->pw_dir, 1); // set the HOME environment variable to the real home directory
#endif
}

#if 0
void createConsole()
{
#ifdef Q_OS_WIN32
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
#endif

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
			fprintf( stderr, "%s", DEPENDENCY_MISSING);
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
			fprintf( stderr, "%s", DEPENDENCY_MISSING);
			return -1;
		}
		LogFileUtils::getInstance()->open(Settings::getInstance()->getLogFileAbsolutePath(), Settings::getInstance()->getMaxLogLines());
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
			fprintf( stderr, "%s", DEPENDENCY_MISSING);
			return -1;
		}
		if ( !assertSingleApplication() )
		{
			fprintf( stderr, "%s", SINGLE_APPLICATION_ERROR);
			return -1;
		}
		LogFileUtils::getInstance()->open(Settings::getInstance()->getLogFileAbsolutePath(), Settings::getInstance()->getMaxLogLines());
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
			fprintf( stderr, "%s", DEPENDENCY_MISSING);
			return -1;
		}
		if ( !assertSingleApplication() )
		{
			fprintf( stderr, "%s", SINGLE_APPLICATION_ERROR);
			return -1;
		}
		LogFileUtils::getInstance()->open(Settings::getInstance()->getLogFileAbsolutePath(), Settings::getInstance()->getMaxLogLines());
		qInstallMsgHandler(messageHandler);
		CliManager::runSchedule();
		app.exit();
		LogFileUtils::getInstance()->close();
		return 0;
	}

	// run with gui
	QApplication app(argc, argv);
	qInstallMsgHandler(messageHandler);
	if ( !initSettings( &app) || !assertCliDependencies())
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
	LogFileUtils::getInstance()->open(Settings::getInstance()->getLogFileAbsolutePath(), Settings::getInstance()->getMaxLogLines());
	MainModel model;

	Settings* settings = Settings::getInstance();

	QTranslator appTranslator, qtTranslator;
	appTranslator.load("app_" + settings->getLanguage(), settings->getApplicationBinDir());
    qtTranslator.load("qt_" + settings->getLanguage(), settings->getApplicationBinDir());
	app.installTranslator(&appTranslator);
	app.installTranslator(&qtTranslator);

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
