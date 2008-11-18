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

#ifndef LOG_FILE_UTILS_HH
#define LOG_FILE_UTILS_HH

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QStringList>

/**
 * The LogFileUtils class provides convenience methods for reading and writing the
 * user log file
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: dtschan $ $Date: 2008/06/30 06:00:01 $ $Revision: 1.6 $
 */
class LogFileUtils : public QObject
{
	Q_OBJECT

public slots:

	/**
	 * Writes the given message to the log file
	 * @param message text to write to the log file
	 */
	void writeLog( const QString& message );

public:

	/**
	 * Gets a LogFileUtils instance
	 * @return a LogFileUtils instance
	 */
	static LogFileUtils* getInstance();

	/**
	 * Opens the log file
	 */
	void open();

	/**
	 * Gets a list of new log lines
	 */
	QStringList getNewLines();

	/**
	 * Closes the log file
	 */
	void close();

	static void logToHex( const QString& string );

	static void logToHex( const QByteArray& data );

private:
	LogFileUtils( const QString& logfilePath, int maxLines );
	virtual ~LogFileUtils();

	static LogFileUtils* instance;
	QFile logFile;
	int maxLines;
	QTextStream output;
	QStringList newLines;
};
#endif
