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

#include <cassert>

#include <QTextCodec>
#include <QLocale>
#include <QDebug>
#include <QProcess>

#include "settings/settings.hh"
#include "tools/process.hh"
#include "utils/log_file_utils.hh"

const int Process::MAX_LINE_SIZE = 60000;

Process::Process() :
	qProcess(0)
{
}

Process::~Process()
{
	terminate();
}

void Process::createProcess(const QString& executableName)
{
	QStringList arguments;
	createProcess(executableName, arguments);
}

void Process::createProcess(const QString& executableName, const QStringList& arguments)
{
	terminate();
	qProcess = new QProcess;
	this->executableName = executableName;
	this->arguments = arguments;
}

void Process::start() throw (ProcessException )
{
	assert(qProcess);
	QString log_msg = "";
	log_msg = log_msg.append("Started: ").append(executableName).append(" ").append(arguments.join(" "));
	qDebug() << "Started: " << executableName << arguments;
	LogFileUtils::getInstance()->writeLog(log_msg);

	qProcess->start(executableName, arguments);
	if (qProcess->waitForStarted() )
	{
		qDebug() << "         " << executableName << " started successfully.";
		LogFileUtils::getInstance()->writeLog("         " + executableName + " started successfully.");
		return;
	}
	qCritical() << "Failed: " << executableName << arguments;
	throw ProcessException( QObject::tr( "The process %1 failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program." ).arg( executableName ) );
}

void Process::setWorkingDirectory(const QString& directory)
{
	assert(qProcess);
	qProcess->setWorkingDirectory(directory);
}

bool Process::blockingReadLine(QByteArray* byteArray, int msec)
{
	assert(qProcess);
	while ( !qProcess->canReadLine() )
	{
		if ( !qProcess->waitForReadyRead(msec) )
		{
			if (qProcess->error() == QProcess::Timedout)
			{
				qWarning() << "Process timedout after " << msec << "msec";
				return false;
			}
			else if (qProcess->state() == QProcess::NotRunning)
			{
				return false;
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
}

bool Process::blockingReadLine(QString* string, int msec)
{
	QByteArray line;
	bool result = blockingReadLine( &line, msec);
	if (Settings::IS_WINDOWS)
	{
		*string = QString::fromUtf8(line);
	}
	else
	{
		QString inputString = QString::fromLocal8Bit(line);
		*string = inputString.normalized(QString::NormalizationForm_C);
	}
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
	if (qProcess)
	{
		qProcess->kill();
		qProcess->waitForFinished(10 * 1000);
		delete qProcess;
		qProcess = 0;
	}
}

