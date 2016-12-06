/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2011 stepping stone GmbH
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

#ifndef PROCESS_HH
#define PROCESS_HH

#include <cassert>
#include <QTextStream>
#include <QWaitCondition>

#include "exception/process_exception.hh"
#include "tools/abstract_informing_process.hh"
#include "tools/extended_qprocess.hh"

/**
 * The Process class provides the basic implementation for starting an external program
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Process
{
public:

	/**
	 * Constructs a Process
	 */
	Process();

	/**
	 * Destroys the Process
	 */
	virtual ~Process();

protected:

    /**
     * Creates a new process for the given external program with the given arguments
     * @param executableName name of the external program
     * @param arguments list of arguments for the external program
     * @param filteredEnvVars list of environment variables, which should be removed for this process.
     */
    void createProcess(const QString& executableName, const QStringList& arguments,
                                                  const QList<QString>& filteredEnvVars);

	/**
	 * Creates a new process for the given external program with the given arguments
	 * @param executableName name of the external program
	 * @param arguments list of arguments for the external program
	 */
	void createProcess( const QString& executableName, const QStringList& arguments );

	/**
	 * Creates a new process for the given external program
	 * @param executableName name of the external program
	 */
	void createProcess( const QString& executableName );

	/**
	 * Sets the working directory for the process
	 * @param directory name of the directory
	 */
	void setWorkingDirectory( const QString& directory );

	/**
	 * Starts the created process
	 * @return true if start was successful
	 * @see Process::createProcess( const QString& executableName, const QStringList& arguments )
	 * @see Process::createProcess( const QString& executableName )
	 */
	void start();

	/**
	 * Reads a line of data and stores the characters in byteArray. This method blocks until a line
	 * can be read. If an error occured, or if there is no more line to read, it returns false.
	 * @param byteArray a byte array to store the characters into
	 * @param msec milliseconds to block
	 */
	bool blockingReadLine(QByteArray* byteArray, int msec = 30000, char lineEndChar = '\n');

	/**
	 * Reads a line of data and stores the characters in string. This method blocks until a line
	 * can be read. If an error occurred, or if there is no more line to read, it returns false.
	 * @param string a string to store the characters into
	 * @param msec milliseconds to block
	 */
	bool blockingReadLine( QString* string, int msec = 30000, char lineEndChar = '\n' );

	/**
	 * Blocks until the process has finished
	 */
	bool waitForFinished(int msecs = 60 * 60 * 1000);

	/**
	 * Logs all standard error and standard output
	 */
	void logAll();

	/**
	 * Terminates the process
	 */
	void terminate();

	void retrieveStream(QTextStream* textStream) const;

	QByteArray readAllStandardError();
	QByteArray readAllStandardOutput();
	bool waitForReadyRead();
	qint64 bytesAvailable();
	qint64 write ( const QByteArray& byteArray );
	void setProcessChannelMode ( QProcess::ProcessChannelMode mode );
	QByteArray readAll();
	int exitCode() const;
	QProcess::ExitStatus exitStatus() const;
	QByteArray readLine( qint64 maxSize = 0 );
	void setStandardInputFile( const QString & fileName );
	bool waitForReadyRead( int msecs );
	void setStandardOutputFile( const QString & fileName, QIODevice::OpenMode mode = QIODevice::Truncate );
	bool waitForStarted( int msecs = 30000 );
	void closeWriteChannel();
	bool waitForBytesWritten();
	void setTextModeEnabled( bool enabled );
	void setReadChannel(QProcess::ProcessChannel channel);
	QProcess::ProcessState state() const;
	bool isAlive();

private:
	/**
	 * A QProcess instance
	 */
	ExtendedQProcess* qProcess;
	QString readBuffer;
	bool abort;
	bool killed;
	QWaitCondition abortCondition;

	static const int MAX_LINE_SIZE;
	QString executableName;
	QStringList arguments;
    QList<QString> filteredEnvironmentVars;
};

inline QByteArray Process::readAllStandardError()
{
	assert(qProcess);
	return qProcess->readAllStandardError();
}

inline QByteArray Process::readAllStandardOutput()
{
	assert(qProcess);
	return qProcess->readAllStandardOutput();
}

inline bool Process::waitForReadyRead()
{
	assert(qProcess);
	return qProcess->waitForReadyRead();
}

inline qint64 Process::bytesAvailable()
{
	assert(qProcess);
	return qProcess->bytesAvailable();
}

inline qint64 Process::write( const QByteArray& byteArray )
{
	assert(qProcess);
	return qProcess->write(byteArray);
}

inline void Process::setProcessChannelMode( QProcess::ProcessChannelMode mode )
{
	assert(qProcess);
	qProcess->setProcessChannelMode(mode);
}

inline QByteArray Process::readAll()
{
	assert(qProcess);
	return qProcess->readAll();
}

inline int Process::exitCode() const
{
	assert(qProcess);
	return qProcess->exitCode();
}

inline QProcess::ExitStatus Process::exitStatus() const
{
	assert(qProcess);
	return qProcess->exitStatus();
}

inline QByteArray Process::readLine( qint64 maxSize )
{
	assert(qProcess);
	return qProcess->readLine(maxSize);
}

inline void Process::setStandardInputFile( const QString & fileName )
{
	assert(qProcess);
	qProcess->setStandardInputFile(fileName);
}

inline bool Process::waitForReadyRead( int msecs )
{
	assert(qProcess);
	return qProcess->waitForReadyRead(msecs);
}

inline void Process::setStandardOutputFile( const QString & fileName, QIODevice::OpenMode mode )
{
	assert(qProcess);
	qProcess->setStandardOutputFile(fileName, mode);
}

inline bool Process::waitForStarted( int msecs )
{
	assert(qProcess);
	return qProcess->waitForStarted(msecs);
}

inline void Process::closeWriteChannel()
{
	assert(qProcess);
	qProcess->closeWriteChannel();
}

inline bool Process::waitForBytesWritten()
{
	assert(qProcess);
	return qProcess->waitForBytesWritten();
}

inline void Process::setTextModeEnabled ( bool enabled )
{
	assert(qProcess);
	qProcess->setTextModeEnabled(enabled);
}

inline void Process::retrieveStream(QTextStream* textStream) const
{
	assert(qProcess);
	textStream->setDevice(qProcess);
}

inline bool Process::waitForFinished(int msecs)
{
	if (this->isAlive())
	{
		return qProcess->waitForFinished(msecs);
	}
	else
	{
		return true;
	}
}

inline void Process::setReadChannel(QProcess::ProcessChannel channel)
{
	assert(qProcess);
	qProcess->setReadChannel(channel);
}

inline QProcess::ProcessState Process::state() const
{
	assert(qProcess);
	return qProcess->state();
}

#endif

