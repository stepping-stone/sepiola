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

#include <algorithm>
#include <stdexcept>
#include <iostream>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "utils/log_file_utils.hh"
#include "test/test_manager.hh"
#include "settings/settings.hh"

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
	if ( !instance )
	{
		Settings* settings = Settings::getInstance();
		instance = new LogFileUtils( settings->getLogFileAbsolutePath(), settings->getMaxLogLines() );
	}
	return instance;
}

LogFileUtils::LogFileUtils( const QString& logfilePath, int maxLines ) : logFile(logfilePath)
{
	this->maxLines = maxLines;
}

void LogFileUtils::open()
{
	if ( !logFile.open( QIODevice::ReadWrite ) )
	{
		fprintf(stderr, "Can not write to log file " + logFile.fileName().toUtf8() );
		return;
	}

	// read the logfile line by line
	QStringList existingLines;
	QTextStream input( &logFile );
	while ( !input.atEnd() )
	{
		existingLines << input.readLine();
	}
	logFile.close();


	// truncate the log file if necessary
	while ( existingLines.size() >= maxLines )
	{
		existingLines.removeAt( 0 );
	}

	if ( !logFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		fprintf( stderr, "Can not truncate log file " + logFile.fileName().toUtf8() );
	}
	output.setDevice( &logFile );
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
	QStringList result = newLines;
	newLines.clear();
	return result;
}

void LogFileUtils::close()
{
	logFile.close();
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
