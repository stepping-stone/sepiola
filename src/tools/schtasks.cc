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

#include <QDebug>
#include <QLocale>
#include <QDialog>
#include <QTextCodec>
#include <QStringList>
#include <QRegExp>
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#include "tools/schtasks.hh"
#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "cli/cli_manager.hh"
#include "utils/log_file_utils.hh"
#include "model/scheduled_task.hh"

const QString Schtasks::SCHTASKS_NAME = "schtasks";

Schtasks::Schtasks()
{
}

Schtasks::~Schtasks()
{
}

void Schtasks::scheduleTask( const QString& execName, const QString& cliArgument, const int& minutesToDelay )
{
	qDebug() << "Schtasks::setScheduleTask( " << execName << ", " << cliArgument << ", " << minutesToDelay << " )";
	Settings* settings = Settings::getInstance();
	QLocale locale = getSchtasksBinaryLocale();

	QStringList schtasksArguments;
	schtasksArguments << "/create";
	schtasksArguments << "/tn";
	schtasksArguments << settings->getApplicationName();
	schtasksArguments << "/tr";
	schtasksArguments << "\"" + execName + "\" " + cliArgument;
	schtasksArguments << "/sc";
	schtasksArguments << getTranslationForOnStart( locale );
	if ( schedule( execName, cliArgument, schtasksArguments, locale ) )
	{
		ScheduledTask sr = settings->getScheduleRule();
		sr.setMinutesAfterStartup( minutesToDelay );
		settings->saveScheduleRule(sr);
	}
}

void Schtasks::scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] )
{
	qDebug() << "Schtasks::setScheduleTask( " << execName << ", " << cliArgument << ", " << time << ", days )";
	Settings* settings = Settings::getInstance();
	QLocale locale = getSchtasksBinaryLocale();
	QString dayString;
	QSet<ScheduledTask::WeekdaysEnum> wd;
	for ( int i = 0; i < 7; i++ )
	{
		if ( days[i] )
		{
			wd.insert((ScheduledTask::WeekdaysEnum)i);
			if ( dayString != "" )
			{
				dayString.append( "," );
			}
			dayString.append( locale.dayName( i + 1, QLocale::ShortFormat ) );
		}
	}

	QStringList schtasksArguments;
	schtasksArguments << "/create";
	schtasksArguments << "/tn" << settings->getApplicationName();
	schtasksArguments << "/tr" << "\"" + execName + "\" " + cliArgument;
	schtasksArguments << "/sc" << getTranslationForWeekly( locale );
	schtasksArguments << "/st" << time.toString( "HH:mm:ss" );
	schtasksArguments << "/d" << dayString;
	schedule( execName, cliArgument, schtasksArguments, locale );
	
	settings->saveScheduleRule(ScheduledTask(wd, time));
}

bool Schtasks::schedule( const QString& execName, const QString& cliArgument, const QStringList& schtasksArguments,
																									const QLocale& locale )
{
	qDebug() << "Schtasks::schedule( " << execName << ", " << cliArgument << ", " << schtasksArguments << ", " << locale << " )";

	QString username = "";
#ifdef Q_OS_WIN32
	username = QString::fromWCharArray( _wgetenv( L"USERDOMAIN" ) ) + "\\" + QString::fromWCharArray( _wgetenv( L"USERNAME" ) );
#endif

	QString password;
	bool schtasks_success = false;
	bool password_ok = getWindowsPassword( username, password, QObject::tr( "Enter login information to create the schedule task." ) );

	if ( password_ok )
	{
		QStringList arguments = schtasksArguments;
		arguments << "/ru" << username;
		//arguments << "/rp" << password;

		deleteExistingTask( execName, cliArgument );
		createProcess( SCHTASKS_NAME, arguments );
		setProcessChannelMode( QProcess::MergedChannels );
		start(); // this may cause an error if no job has been created before. let's ignore this message
		waitForReadyRead();
		readAll();
		write(( password + "\r\n" ).toUtf8() );
		waitForBytesWritten();
		schtasks_success = true;

		QString lineData;
		waitForFinished();
		QString codecName = getCodecName();
		QTextCodec* codec = 0;
		if (( codec = QTextCodec::codecForName( codecName.toLatin1() ) ) != 0 || ( codec = QTextCodec::codecForName(( "CP_" + codecName ).toLatin1() ) ) != 0 )
		{
			lineData = codec->toUnicode( readAll() );
		}
		else
		{
			lineData = QString::fromLocal8Bit( readAll() );
		}
		if ( lineData.contains( getTranslationForWARNING( locale ) ) )
		{
			schtasks_success = false;
			QString warning = lineData.right( lineData.size()
																																					- lineData.indexOf( getTranslationForWARNING( locale ) ) );
			qDebug() << warning;
			LogFileUtils::getInstance()->writeLog( warning );
		}
		if ( lineData.contains( getTranslationForERROR( locale ) ) )
		{
			schtasks_success = false;
			QString error = lineData.right( lineData.size()
																																			- lineData.indexOf( getTranslationForERROR( locale ) ) );
			qDebug() << error;
			LogFileUtils::getInstance()->writeLog( error );
		}
	}
	if ( schtasks_success )
	{
		emit infoSignal( QObject::tr( "Scheduled task has been created" ) );
	}
	else
	{
		emit infoSignal( QObject::tr( "Scheduled task could not be created" ) );
	}
	return schtasks_success;
}

bool Schtasks::taskExists( QString taskName )
{
	Settings* settings = Settings::getInstance();
	if ( taskName == "" )
	{
		taskName = settings->getApplicationName();
	}

	QStringList arguments;
	arguments << "/query";
	arguments << "/fo" << "csv";
	createProcess( SCHTASKS_NAME, arguments );
	start();

	waitForFinished();
	QString lineData;
	if ( bytesAvailable() > 0 )
	{
		lineData = QString::fromLocal8Bit( readAll() );
		if ( lineData.contains( "\"" + taskName + "\"" ) )
		{
			return true;
		}
	}
	return false;
}

bool Schtasks::updateExistingTask( const QString& execName, const QString& cliArgument )
{
	qDebug() << "Schtasks::updateExistingTask( " << execName << ", " << cliArgument << " )";
	Settings* settings = Settings::getInstance();
	QString taskName = settings->getApplicationName();
	bool schtasks_success = false;

	if ( taskExists() )
	{
		QLocale locale = getSchtasksBinaryLocale();
		QString username = "";
#ifdef Q_OS_WIN32
		username = QString::fromWCharArray( _wgetenv( L"USERDOMAIN" ) ) + "\\" + QString::fromWCharArray( _wgetenv( L"USERNAME" ) );
#endif
		QString password;
		bool password_ok = getWindowsPassword( username, password, QObject::tr( "Enter login information to update the existing schedule task." ) );

		if ( password_ok )
		{
			QStringList arguments;
			arguments << "/change";
			arguments << "/tn" << taskName;
			arguments << "/tr" << "\"" + execName + "\" " + cliArgument;
			arguments << "/ru" << username;
			//arguments << "/rp" << password;

			createProcess( SCHTASKS_NAME, arguments );
			setProcessChannelMode( QProcess::MergedChannels );
			start(); // this may cause an error if no job has been created before. let's ignore this message
			waitForReadyRead();
			readAll();
			write(( password + "\r\n" ).toUtf8() );
			waitForBytesWritten();
			schtasks_success = true;

			QString lineData;
			waitForFinished();
			QString codecName = getCodecName();
			QTextCodec* codec = 0;
			if (( codec = QTextCodec::codecForName(( "CP" + codecName ).toLatin1() ) ) != 0 || ( codec = QTextCodec::codecForName( codecName.toLatin1() ) ) != 0 )
			{
				lineData = codec->toUnicode( readAll() );
			}
			else
			{
				lineData = QString::fromLocal8Bit( readAll() );
			}
			if ( lineData.contains( getTranslationForWARNING( locale ) ) )
			{
				schtasks_success = false;
				QString warning = lineData.right( lineData.size()
																																						- lineData.indexOf( getTranslationForWARNING( locale ) ) );
				qDebug() << warning;
				LogFileUtils::getInstance()->writeLog( warning );
			}
			if ( lineData.contains( getTranslationForERROR( locale ) ) )
			{
				schtasks_success = false;
				QString error = lineData.right( lineData.size()
																																				- lineData.indexOf( getTranslationForERROR( locale ) ) );
				qDebug() << error;
				LogFileUtils::getInstance()->writeLog( error );
			}
		}
		if ( schtasks_success )
		{
			emit infoSignal( QObject::tr( "Scheduled task has been updated" ) );
		}
		else
		{
			emit infoSignal( QObject::tr( "Scheduled task could not be updated" ) );
		}
		return schtasks_success;
	}
	else
	{
		QString info = "there is no task to update";
		qDebug() << info;
		LogFileUtils::getInstance()->writeLog( info );
	}
	return schtasks_success;
}

void Schtasks::deleteExistingTask( const QString& taskName )
{
	qDebug() << "Schtasks::deleteExistingTask( " << taskName << " )";
	// remove existing backup job entry
	QStringList arguments;
	arguments << "/delete";
	arguments << "/tn" << taskName;
	arguments << "/f";
	QString lineData;

	createProcess( SCHTASKS_NAME, arguments );
	start(); // this may cause an error if no job has been created before. let's ignore this message
	waitForFinished();
}

void Schtasks::deleteExistingTask( const QString& execName, const QString& cliArgument )
{
	// ignoring the passed cliArgument, deleting SCHTASKS based on settings->getApplicationName()
	qDebug() << "Schtasks::deleteExistingTask( " << execName << ", " << cliArgument << " )";
	Settings* settings = Settings::getInstance();
	deleteExistingTask( settings->getApplicationName() );
}

bool Schtasks::getWindowsPassword( const QString& username, QString& password, const QString& msg )
{
	qDebug() << "Schtasks::getWindowsPassword( " << username << ", " << password << " )";
	Settings* settings = Settings::getInstance();

	QLocale locale = getSchtasksBinaryLocale();
	QString codecName = getCodecName();
	const QString cliArgument = "-noop";
	const QString taskName = "Temporary " + settings->getApplicationName() + " Task";
	QStringList schtasksArguments;
	schtasksArguments << "/create";
	schtasksArguments << "/tn" << taskName;
	schtasksArguments << "/tr" << "\"" + settings->getApplicationName() + "\"";
	schtasksArguments << "/sc" << getTranslationForOnStart( locale );

	bool schtasks_success = false;
	bool requery_password = true;
	while ( requery_password )
	{
		deleteExistingTask( taskName );
		if ( settings->getClientPassword().length() == 0 )
		{
			int result;
			emit askForPassword( username, false, &result, msg );
			requery_password = ( result == QDialog::Accepted );
		}
		if ( requery_password )
		{
			QStringList arguments = schtasksArguments;
			arguments << "/ru" << username;
			//arguments << "/rp" << settings->getClientPassword().toLatin1();

			createProcess( SCHTASKS_NAME, arguments );
			setProcessChannelMode( QProcess::MergedChannels );
			start(); // this may cause an error if no job has been created before. let's ignore this message
			waitForReadyRead();
			//readAll();
			write(( settings->getClientPassword() + "\r\n" ).toLatin1() );
			waitForBytesWritten();

			requery_password = false;
			schtasks_success = true;

			QString lineData;
			waitForFinished();
			QTextCodec* codec = 0;
			if (( codec = QTextCodec::codecForName( codecName.toLatin1() ) ) != 0 || ( codec = QTextCodec::codecForName(( "CP_" + codecName ).toLatin1() ) ) != 0 )
			{
				lineData = codec->toUnicode( readAll() );
			}
			else
			{
				lineData = QString::fromLocal8Bit( readAll() );
			}
			if ( lineData.contains( getTranslationForWARNING( locale ) ) )
			{
				schtasks_success = false;
				settings->setClientPassword( "" );
				LogFileUtils::getInstance()->writeLog( lineData );
				if ( lineData.contains( getTranslationForWrongAccountInformation( locale ) ) )
				{
					requery_password = true;
					QString warning = "wrong password -> requery password";
					qDebug() << warning;
					LogFileUtils::getInstance()->writeLog( warning );
				}
			}
			if ( lineData.contains( getTranslationForERROR( locale ) ) )
			{
				schtasks_success = false;
				settings->setClientPassword( "" );
				QString error = lineData.right( lineData.size()
																																				- lineData.indexOf( getTranslationForERROR( locale ) ) );
				qDebug() << error;
				LogFileUtils::getInstance()->writeLog( error );
			}
		}
	}
	deleteExistingTask( taskName );
	if ( schtasks_success )
	{
		password = settings->getClientPassword().toLatin1();
		return true;
	}
	else
	{
		settings->setClientPassword( "" );
		return false;
	}
}

QLocale Schtasks::getSchtasksBinaryLocale()
{
	qDebug() << "Schtasks::getLocale()";

	QStringList arguments;
	arguments << "/?";
	createProcess( SCHTASKS_NAME, arguments );
	setProcessChannelMode( QProcess::MergedChannels );
	start();

	waitForReadyRead();
	waitForFinished();
	QString lineData;
	QString codecName = getCodecName();
	QTextCodec* codec = 0;
	if (( codec = QTextCodec::codecForName( codecName.toLatin1() ) ) != 0 || ( codec = QTextCodec::codecForName(( "CP_" + codecName ).toLatin1() ) ) != 0 )
	{
		lineData = codec->toUnicode( readAll() );
	}
	else
	{
		lineData = QString::fromLocal8Bit( readAll() );
	}
	if ( lineData.contains( "SCHTASKS /Parameter [Argumente]" ) )
	{
		return QLocale( QLocale::German );
	}
	if ( lineData.endsWith( "SCHTASKS /param\u00E8tre [arguments]" ) )
	{
		return QLocale( QLocale::French );
	}
	return QLocale( QLocale::English );
}

QString Schtasks::getCodecName()
{
	QString codecName = "";
#ifdef Q_OS_WIN32
	char oemCodePage[6];
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, oemCodePage, sizeof( oemCodePage ) / sizeof( char ) );
	codecName = QString( oemCodePage );
#endif
	return codecName;
}

QString Schtasks::getTranslationForWrongAccountInformation( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return QString::fromWCharArray( L"wurde erstellt, aber er kann nicht ausgef\u00FChrt werden, weil die Kontoinformationen nicht festgelegt werden konnten." );
		default:
			break;
	}
	return "has been created, but may not run because the account information could not be set.";
}

QString Schtasks::getTranslationForPassword( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return "Kennwort";
		case QLocale::French:
			return "mot de passe";
		default:
			break;
	}
	return "password";
}

QString Schtasks::getTranslationForWARNING( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return "WARNUNG";
		case QLocale::French:
			return "AVERTISSEMENT";
		default:
			break;
	}
	return "WARNING";
}

QString Schtasks::getTranslationForERROR( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return "FEHLER";
		case QLocale::French:
			return "ERREUR";
		default:
			break;
	}
	return "ERROR";
}

QString Schtasks::getTranslationForWeekly( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return QString::fromWCharArray( L"w\u00F6chentlich" );
		default:
			break;
	}
	return "weekly"; // also true for french
}

QString Schtasks::getTranslationForOnStart( const QLocale& locale )
{
	switch ( locale.language() )
	{
		case QLocale::German:
			return "beimstart";
		default:
			break;
	}
	return "onstart"; // also true for french
}

bool Schtasks::isSchtasksSupported()
{
	QProcess process;
	QStringList arguments;
	arguments << "/?";
	process.start( SCHTASKS_NAME, arguments );
	bool result = process.waitForStarted();
	process.waitForFinished();
	return result;
}

bool Schtasks::isSchedulingOnStartSupported()
{
	return true;
}

void Schtasks::testSchtasksScheduleTask()
{
	Schtasks schtasks;
	QTime time( 15, 51 );
	bool days[7] =
		{ true, false, true, false, true, true, false };
	try
	{
		schtasks.scheduleTask( "c:\\program files\\sepiola 0.3.0\\bin\\sepiola.exe", CliManager::SCHEDULE_ARGUMENT, time, days );
	}
	catch ( ProcessException e )
	{
		qWarning() << QString( e.what() );
	}
}

namespace
{
	int dummy1 = TestManager::registerTest( "testSchtasksScheduleTask", Schtasks::testSchtasksScheduleTask );
}
