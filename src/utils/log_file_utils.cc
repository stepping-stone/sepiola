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

#include <algorithm>
#include <stdexcept>
#include <iostream>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "utils/log_file_utils.hh"
#include "test/test_manager.hh"

using std::copy;
using std::back_inserter;
using std::runtime_error;
using std::string;
using std::cout;
using std::hex;
using std::dec;
using std::endl;

LogFileUtils* LogFileUtils::instance = 0;

LogFileUtils* LogFileUtils::getInstance()
{
	QMutex mutex;
	QMutexLocker locker(&mutex);

	if ( !instance )
		instance = new LogFileUtils;

	return instance;
}

LogFileUtils::LogFileUtils() :
    logFile(NULL)
{
}

void LogFileUtils::open( const QString& logfilePath, int maxLines )
{
	QMutexLocker locker(&mutex);

    if (logFile)
        close();

    logFile = new QFile(logfilePath);

	if ( !logFile->open( QIODevice::ReadWrite ) )
	{
		fprintf(stderr, "Can not write to log file %s", logfilePath.toUtf8().data() );
		return;
	}

	// read the logfile line by line
	QStringList existingLines;
	QTextStream input( logFile );
	while ( !input.atEnd() )
	{
		existingLines << input.readLine();
	}
	logFile->close();


	// truncate the log file if necessary
	while ( existingLines.size() >= maxLines )
	{
		existingLines.removeAt( 0 );
	}

	if ( !logFile->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		fprintf( stderr, "Can not truncate log file %s", logfilePath.toUtf8().data() );
	}
	output.setDevice( logFile );
	{
		foreach( QString line, existingLines )
		{
			newLines << line;
			output << line << endl;
		}
	}
	output.flush();
}

LogFileUtils::~LogFileUtils()
{
	close();
}

void LogFileUtils::writeLog( const QString& message )
{
	static const QChar* lastMessage = 0;

	QMutexLocker locker(&mutex);

	// Filter duplicate messages
	if (message.constData() == lastMessage) return;
	lastMessage = message.constData();

	QString dateTime = QDateTime::currentDateTime ().toString( "dd.MM.yy hh:mm:ss");
	QString line;
	line.append( dateTime );
	line.append( "\t" );
	line.append( message );
	newLines << line;
	output << line << endl;
	output.flush();
}

QStringList LogFileUtils::getNewLines()
{
	QMutexLocker locker(&mutex);
	QStringList result = newLines;
	newLines.clear();
	return result;
}

void LogFileUtils::close()
{
	QMutexLocker locker(&mutex);

    if (logFile)
    {
    	logFile->close();
        delete logFile;
        logFile = NULL;
    }
}

void LogFileUtils::logToHex( const QString& string )
{
	qDebug() << "LogFileUtils::logToHex( " << string << " )";

	const ushort* test = string.utf16();
	cout << hex;
	while( *test )
	{
		cout << hex << *test++ << " ";
	}
	cout << endl;
	cout << dec;
}

void LogFileUtils::logToHex( const QByteArray& data )
{
	qDebug() << "LogFileUtils::logToHex( " << data << " )";

	const char* test = data.data();
	cout << hex;
	while( *test )
	{
		cout << (ushort) *test++ << " ";
	}
	cout << endl;
	cout << dec;
}
