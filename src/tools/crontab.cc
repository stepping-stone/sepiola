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
#include <QStringList>
#include <QTemporaryFile>

#include "tools/crontab.hh"
#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "cli/cli_manager.hh"

const QString Crontab::CRONTAB_NAME = "crontab";

Crontab::Crontab()
{
}

Crontab::~Crontab()
{
}

void Crontab::scheduleTask( const QString& execName, const QString& cliArgument, const int& minutesToDelay )
{
	qDebug() << "Crontab::setScheduleTask( " << execName << ", " << cliArgument << ", " << minutesToDelay << " )";

	if ( !isVixieCron() )
	{
		emit errorSignal( QObject::tr( "Scheduling failed, because no vixie cron has been found" ) );
		return;
	}

	if ( schedule( execName, cliArgument, "@reboot" ) )
	{
		Settings::getInstance()->saveSchedulerDelay( minutesToDelay );
	}
}

void Crontab::scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] )
{
	qDebug() << "Crontab::setScheduleTask( " << execName << ", " << cliArgument << ", " << time << ", days )";

	QString dayString;
	for ( int i=0; i<7; i++ )
	{
		if ( days[i] )
		{
			if ( dayString != "" )
			{
				dayString.append( "," );
			}
			dayString.append( QVariant( i+1 ).toString() );
		}
	}
	QString timeSpecification;
	timeSpecification.append( QVariant( time.minute() ).toString() );
	timeSpecification.append( " " );
	timeSpecification.append( QVariant( time.hour() ).toString() );
	timeSpecification.append( " * * " );
	timeSpecification.append( dayString );
	schedule( execName, cliArgument, timeSpecification );
	Settings::getInstance()->saveSchedulerDelay( 0 );
}

bool Crontab::updateExistingTask( const QString& execName, const QString& cliArgument )
{
	if (taskExists(execName, cliArgument)) {
		qDebug() << "Crontab::updateExistingTask( " << execName << ", " << cliArgument << " )";
		QStringList entries = readAllCrontabEntries();
		entries = updateExistingBackupJob( entries, execName, cliArgument );
		writeAllCrontabEntries( entries );
		return true;
	}
	return false;
}

void Crontab::deleteExistingTask( const QString& execName, const QString& cliArgument )
{
	qDebug() << "Crontab::deleteExistingTask( " << cliArgument << " )" ;
	QStringList entries = readAllCrontabEntries();
	entries = removeExistingBackupJob( entries, execName, cliArgument );
	writeAllCrontabEntries( entries );
}

QStringList Crontab::readAllCrontabEntries()
{
	qDebug() << "Crontab::readAllCrontabEntries()" ;
	// read old crontab entries
	QStringList arguments( "-l" );
	createProcess( CRONTAB_NAME, arguments );
	start();

	QStringList oldCrontabEntries;

	QByteArray line;
	while ( blockingReadLine( &line ) )
	{
		line.replace( "\n", "" );
		oldCrontabEntries << line;
	}
	return oldCrontabEntries;
}

bool Crontab::writeAllCrontabEntries( const QStringList& crontabEntries )
{
	qDebug() << "Crontab::writeAllCrontabEntries( " << crontabEntries << " )";
	Settings* settings = Settings::getInstance();
	// write jobs line by line to a temporary file
	// the QTemporaryFile destructor removes the temporary crontabFile
	QTemporaryFile crontabFile;
	if ( !crontabFile.open() )
	{
		emit errorSignal( QObject::tr( "Scheduling failed, because file %1 could not be written" ).arg( crontabFile.fileName() ) );
		return false;
	}
	QTextStream out( &crontabFile );
	foreach( QString crontabEntry, crontabEntries )
	{
		out << crontabEntry;
		out << settings->getEOLCharacter();
	}
	out.flush();

	QStringList arguments;
	arguments << crontabFile.fileName();
	createProcess( CRONTAB_NAME, arguments );
	start();
	waitForFinished();
	QString error = readAllStandardError();
	if ( error == "" )
	{
		emit infoSignal( QObject::tr( "Job has been scheduled." ) );
		return true;
	}
	emit errorSignal( QObject::tr( "Scheduling failed. Reason is: " ) + error );
	return false;
}

bool Crontab::taskExists( const QString& execName,  const QString& cliArgument )
{
	qDebug() << "Crontab::taskExists( " << execName << ", " << cliArgument << " )";
	// check if backup job exists
	QStringList existingEntries = readAllCrontabEntries();
	QString compareString = "\"" + execName + "\" " + cliArgument;
	compareString = compareString.mid(compareString.lastIndexOf("/")+1);
	for ( int i=0; i<existingEntries.size(); i++ )
	{
		QString existingEntry = existingEntries.at( i );
		if ( existingEntry.endsWith( compareString ) )
		{
			return true;
		}
	}
	return false;
}

QStringList Crontab::removeExistingBackupJob( const QStringList& existingEntries, const QString& execName,  const QString& cliArgument )
{
	qDebug() << "Crontab::removeExistingBackupJob( " << existingEntries << ", " << cliArgument << " )";
	// remove possible existing backup job entry
	QStringList result;
	QString compareString = "\"" + execName + "\" " + cliArgument;
	compareString = compareString.mid(compareString.lastIndexOf("/")+1);
	for ( int i=0; i<existingEntries.size(); i++ )
	{
		QString existingEntry = existingEntries.at( i );
		if ( !existingEntry.endsWith( compareString ) )
		{
			result << existingEntry;
		}
	}
	return result;
}

QStringList Crontab::updateExistingBackupJob( const QStringList& existingEntries, const QString& execName, const QString& cliArgument )
{
	qDebug() << "Crontab::updateExistingBackupJob( " << existingEntries << ", " << execName << ", " << cliArgument << " )";
	// update possible existing backup job entry
	QStringList result;
	QString compareString = "\"" + execName + "\" " + cliArgument;
	compareString = compareString.mid(compareString.lastIndexOf("/")+1);
	for ( int i=0; i<existingEntries.size(); i++)
	{
		QString existingEntry = existingEntries.at( i );
		if ( existingEntry.endsWith( compareString ) )
		{
			existingEntry = existingEntry.left( existingEntry.indexOf( "/" ) - 1 );
			existingEntry.append( "\"" + execName + "\" " );
			existingEntry.append( cliArgument );
		}
		result << existingEntry;
	}
	return result;
}

bool Crontab::schedule( const QString& execName, const QString& cliArgument, const QString& timeSpecification )
{
	static const char* LANG_VARS[] = { "LC_ALL", "LC_CTYPE", "LANG" };

	qDebug() << "Crontab::schedule( " << execName << ", " << cliArgument << ", " << timeSpecification << " )";

	QStringList crontabEntries = readAllCrontabEntries();
	crontabEntries = removeExistingBackupJob( crontabEntries, execName, cliArgument );

	QString lang;
	for (size_t i = 0; i < sizeof(LANG_VARS) / sizeof(const char*); i++)
	{
		if (getenv(LANG_VARS[i]) != NULL)
		{
			lang = getenv(LANG_VARS[i]);
			break;
		}
	}

	// add new backup job
	QString newCrontabEntry;
	newCrontabEntry.append( timeSpecification );
	newCrontabEntry.append( " " );
	if (!lang.isEmpty())
	{
		newCrontabEntry.append( "LANG=" );
		newCrontabEntry.append( lang );
		newCrontabEntry.append( " " );
	}
	newCrontabEntry.append( "\"" + execName + "\" " );
	newCrontabEntry.append( cliArgument );

	crontabEntries << newCrontabEntry;
	return writeAllCrontabEntries( crontabEntries );
}

bool Crontab::isVixieCron()
{
	qDebug() << "Crontab::isVixieCron()";
	QString crontabFileName = "/usr/bin/crontab"; //TODO: find real crontab path
	QFile crontabFile( crontabFileName );
	if ( !crontabFile.open( QIODevice::ReadOnly ) )
	{
		qWarning() << "Can not open " << crontabFileName;
		return false;
	}
	if ( crontabFile.size() > 2*1024*1024 ) // if the binary's size is too big, assume that this is a wrong file
	{
		qWarning() << "Crontab binary's size not supported";
		return false;
	}
	QString paulVixieCopyright = "Paul Vixie";
	QByteArray content = crontabFile.readAll();
	if ( content.indexOf( paulVixieCopyright ) > -1 )
	{
		return true;
	}
	return false;
}

bool Crontab::isSchedulingOnStartSupported()
{
	return true;
}

void Crontab::testScheduleTask()
{
	Crontab crontab;
	QTime time( 10, 20 );
	bool days[7] = { true, false, true, false, true, true, false };
	crontab.scheduleTask( "/bin/ls", CliManager::SCHEDULE_ARGUMENT, time, days );
}

void Crontab::testIsVixieCron()
{
	qDebug() << "is vixie: " << Crontab::isVixieCron();
}

namespace
{
	int dummy1 = TestManager::registerTest( "testScheduleTask", Crontab::testScheduleTask );
	int dummy2 = TestManager::registerTest( "testIsVixieCron", Crontab::testIsVixieCron );
}
