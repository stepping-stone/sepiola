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

#include <cassert>

#include <QTextCodec>
#include <QLocale>
#include <QDebug>
#include <QProcess>
#include <QMutex>

#include "settings/settings.hh"
#include "tools/process.hh"
#include "utils/log_file_utils.hh"
#include "utils/string_utils.hh"

const int Process::MAX_LINE_SIZE = 60000;

Process::Process() :
	qProcess(0), abort(false), killed(false)
{
}

Process::~Process()
{
	terminate();
	delete qProcess;
}

void Process::createProcess(const QString& executableName)
{
	QStringList arguments;
	createProcess(executableName, arguments);
}

void Process::createProcess(const QString& executableName, const QStringList& arguments)
{
	terminate();
	delete qProcess;
	abort = false;
	killed = false;
	qProcess = new ExtendedQProcess;
	this->executableName = executableName;
	this->arguments = arguments;
}

void Process::start()
{
	assert(qProcess);

	// As we don't want to log the password, we need to filter it
	QStringList log_arguments = arguments;

	// Iterate over the arguments
	const int arg_size = log_arguments.size();
	for (int i = 0; i < arg_size ; ++i)
	{
	    // If it is the schtask password, filter it
	    if ( ( log_arguments.at(i) == "/rp" ) && ( i + 1 < arg_size ) )
	        log_arguments.replace(i + 1, "[filtered]");
	}

	qDebug() << "Started: " << executableName << log_arguments;
	LogFileUtils::getInstance()->writeLog("Started: " + executableName + " " + log_arguments.join(" "));

	qProcess->start(executableName, arguments);
	if (qProcess->waitForStarted() )
	{
		qDebug() << "         " << executableName << " started successfully.";
		LogFileUtils::getInstance()->writeLog("         " + executableName + " started successfully.");
		return;
	}
	qCritical() << "Failed: " << executableName << log_arguments;
	throw ProcessException( QObject::tr( "The process %1 failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program." ).arg( executableName ) );
}

void Process::setWorkingDirectory(const QString& directory)
{
	assert(qProcess);
	qProcess->setWorkingDirectory(directory);
}

bool Process::blockingReadLine(QByteArray* byteArray, int msec, char lineEndChar)
{
	const int BLOCKREAD_MAX_LINE_SIZE = 4120; // arbitrarily chosen to 4096 plus file-information-prefix

	if (!this->isAlive()) return false;

	QDateTime endTime = QDateTime::currentDateTime().addMSecs(msec);
	if (lineEndChar == '\n') {
		// simple case, where lineEndChar is "\n" -> canReadLine and readLine only support "\n"
		while ( !qProcess->canReadLine() )
		{
			// QProcess at least up to Qt 4.3.5 does not correctly reset error and errorString, reset manually.
			qProcess->resetError();
			if ( !qProcess->waitForReadyRead(100) )
			{
				if (!qProcess->hasError())  // waitForReadyRead returned false but reports no error
				{
					// eighter the process is dead or the read channel has been closed -> abort read operation
					return false;
				}

				if (qProcess->error() == QProcess::Timedout)
				{
					if (QDateTime::currentDateTime() >= endTime)
					{
						qWarning() << "Process timed out after " << msec << "msec";
						return false;
					}

					if (abort)
					{
						abortCondition.wakeAll();
						return false;
					}
				}
				else
				{
					throw ProcessException( "blockingReadLine: " + qProcess->errorString() );
				}
			}
		}
		char buf[MAX_LINE_SIZE ];
		bool result = qProcess->readLine(buf, MAX_LINE_SIZE) >= 0;
		*byteArray = QByteArray(buf);
		return result;
	} else {
		assert(Settings::SHOW_PROGRESS);

		// when need to break _additionally_ to "\n" also at certain lineEndChar (for ex. at "\r" to read progress-info of rsync)
		// more tricky case, where lineEndChar isn't "\n" -> canReadLine and readLine can not be used lonely
		char buf[BLOCKREAD_MAX_LINE_SIZE];
		int nCharsRead = 0;
		bool result = true;
//		QDateTime timeoutTime = QDateTime::currentDateTime().addMSecs(msec);
		// qDebug() << "!readBuffer.contains(QChar(lineEndChar))=" << (!readBuffer.contains(QChar(lineEndChar))) << "!readBuffer.contains(\"\\n\")" << (!readBuffer.contains("\n")) << "!qProcess->canReadLine()" << (!qProcess->canReadLine()) << "BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0" << (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0);
		while ( !readBuffer.contains(QChar(lineEndChar)) && !readBuffer.contains("\n") && !qProcess->canReadLine() && BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0)
		{
			// qDebug() << "!readBuffer.contains(QChar(lineEndChar))=" << (!readBuffer.contains(QChar(lineEndChar))) << "!readBuffer.contains(\"\\n\")" << (!readBuffer.contains("\n")) << "!qProcess->canReadLine()" << (!qProcess->canReadLine()) << "BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0" << (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0);
			if (!this->isAlive()) return false;

			// QProcess at least up to Qt 4.3.5 does not correctly reset error and errorString, reset manually.
			qProcess->resetError();
			if ( !qProcess->waitForReadyRead(100) )
			{
				if (!qProcess->hasError())  // waitForReadyRead returned false but reports no error
				{
					// eighter the process is dead or the read channel has been closed -> abort read operation
					return false;
				}

				if (qProcess->error() == QProcess::Timedout)
				{
					if (QDateTime::currentDateTime() >= endTime)
					{
						qWarning() << "Process timed out after " << msec << "msec";
						return false;
					}

					if (abort)
					{
						abortCondition.wakeAll();
						return false;
					}
				}
				else
				{
					throw ProcessException( "blockingReadLine: " + qProcess->errorString() );
				}
			}
			else
			{
				if (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1 > 0) {
					nCharsRead = qProcess->read(buf, BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1);
					result = nCharsRead >= 0;
					readBuffer.append(QByteArray(buf,nCharsRead));
				}
			}
		}
		if (qProcess->canReadLine()) {
			if (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1 > 1) {
				nCharsRead = qProcess->readLine(buf, BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1);
				result = nCharsRead >= 0;
				readBuffer.append(QByteArray(buf,nCharsRead));
			}
		}
		// qDebug() << "readBuffer:" << readBuffer;
		int pos = std::min((readBuffer.indexOf(QChar(lineEndChar))+(2*BLOCKREAD_MAX_LINE_SIZE)) % (2*BLOCKREAD_MAX_LINE_SIZE), (readBuffer.indexOf("\n")+(2*BLOCKREAD_MAX_LINE_SIZE)) % (2*BLOCKREAD_MAX_LINE_SIZE));
		if (pos > BLOCKREAD_MAX_LINE_SIZE) {
			*byteArray = QByteArray("");
			return false;
		}
		else
		{
			*byteArray = readBuffer.left(pos).toLatin1();
			readBuffer = readBuffer.mid(pos+1);
			return result;
		}
	}
}

bool Process::blockingReadLine(QString* string, int msec, char lineEndChar)
{
	QByteArray line;
	bool result = blockingReadLine( &line, msec, lineEndChar);
	*string = StringUtils::fromLocalEnc(line);

	return result;
}

void Process::logAll()
{
	assert(qProcess);
	qDebug() << "Output: " << qProcess->readAllStandardOutput();
	qCritical() << "Error: " << qProcess->readAllStandardError();
}

void Process::terminate()
{
	if (this->isAlive())
	{
		qDebug() << "Process::terminate()";
		abort = true;
		QMutex mutex;
		mutex.lock();
		abortCondition.wait(&mutex, 500);
#ifdef Q_OS_WIN32
        killed = true;
        qProcess->kill();
#else
        qProcess->terminate();

        int n = 0;
        while ((++n < 20) && qProcess->state() == QProcess::Running) {
            qProcess->waitForFinished(500);
            qDebug() << "waiting...";
        }
        if (qProcess->state() == QProcess::Running) {
            killed = true;
            qProcess->kill();
        }
#endif
	}
}

bool Process::isAlive()
{
	if (qProcess == 0 || qProcess->state() == QProcess::NotRunning || killed) return false;
	if (abort)
	{
		QMutex mutex;
		mutex.lock();
		abortCondition.wait(&mutex, 1000);
		return false;
	}

	return true;
}

