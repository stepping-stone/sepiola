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
#include <QWaitCondition>
#include <QMutex>

#include "settings/settings.hh"
#include "tools/process.hh"
#include "utils/log_file_utils.hh"
#include "utils/string_utils.hh"

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
	qDebug() << "Started: " << executableName << arguments;
	LogFileUtils::getInstance()->writeLog("Started: " + executableName + " " + arguments.join(" "));

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

bool Process::blockingReadLine(QByteArray* byteArray, int msec, char lineEndChar)
{
	//assert(qProcess);
	int BLOCKREAD_MAX_LINE_SIZE = 300;
	if (!this->isAlive()) return false;
	if (lineEndChar == 10) {
		// simple case, where lineEndChar is "\n" -> canReadLine and readLine only support "\n"
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
	} else {
		// when need to break _additionally_ to "\n" also at certain lineEndChar (for ex. at "\r" to read progress-info of rsync)
		// more tricky case, where lineEndChar isn't "\n" -> canReadLine and readLine can not be used lonely
		char buf[BLOCKREAD_MAX_LINE_SIZE];
		int nCharsRead = 0;
		bool result = true;
		while ( !readBuffer.contains(QChar(lineEndChar)) && !readBuffer.contains("\n") && !qProcess->canReadLine() && BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()>0)
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
			else
			{
				//qDebug() << "BLOCKING_READ_LINE: lese aus Buffer";
				//qDebug() << "Puffer ist im Moment (in while) (" << readBuffer.length() << ")" << StringUtils::buf2QString(readBuffer.toAscii().data());
				//qDebug() << "Kann noch" << BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1 << "Zeichen lesen.";
				if (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1 > 0) {
					nCharsRead = qProcess->read(buf, BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1);
					result = nCharsRead >= 0;
					//qDebug() << "buf (orig)=" << StringUtils::buf2QString(buf);
					//qDebug() << "buf (qstr)=" << StringUtils::buf2QString(QString("").append(buf).toAscii().data());
					readBuffer.append(QByteArray(buf,nCharsRead));
				}
				//qDebug() << "Puffer ist im Moment (in while) (" << readBuffer.length() << ")" << StringUtils::buf2QString(readBuffer.toAscii().data());
			}
		}
		if (qProcess->canReadLine()) {
			//qDebug() << "BLOCKING_READ_LINE: lese LINE aus Buffer";
			//qDebug() << "Puffer ist im Moment (nach while) (" << readBuffer.length() << ")" << StringUtils::buf2QString(readBuffer.toAscii().data());
			if (BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1 > 0) {
				nCharsRead = qProcess->readLine(buf, BLOCKREAD_MAX_LINE_SIZE-readBuffer.length()-1);
				result = nCharsRead >= 0;
				//qDebug() << "buf (orig)=" << StringUtils::buf2QString(buf);
				//qDebug() << "buf (qstr)=" << StringUtils::buf2QString(QString("").append(buf).toAscii().data());
				readBuffer.append(QByteArray(buf,nCharsRead));
			}
		}
		int pos = std::min((readBuffer.indexOf(QChar(lineEndChar))+(2*BLOCKREAD_MAX_LINE_SIZE)) % (2*BLOCKREAD_MAX_LINE_SIZE), (readBuffer.indexOf("\n")+(2*BLOCKREAD_MAX_LINE_SIZE)) % (2*BLOCKREAD_MAX_LINE_SIZE));
		//qDebug() << "Puffer ist im Moment (" << readBuffer.length() << ")" << StringUtils::buf2QString(readBuffer.toAscii().data());
		//qDebug() << "Return-Char gefunden bei:" << pos;
		if (pos > BLOCKREAD_MAX_LINE_SIZE) {
			*byteArray = QByteArray(""); 
			//qDebug() << "BLOCKING_READ_LINE: konnte keine ganze Zeile lesen";
			return false;
		}
		else
		{
			//qDebug() << "BLOCKING_READ_LINE: puffer:" << StringUtils::buf2QString(readBuffer);
			// qDebug() << "BLOCKING_READ_LINE: return(" << readBuffer.left(pos) << ")";
			*byteArray = readBuffer.left(pos).toAscii();
			readBuffer = readBuffer.mid(pos+1);
			//qDebug() << "BLOCKING_READ_LINE: puffer:" << StringUtils::buf2QString(readBuffer);
			return result;
		}
	}
}

bool Process::blockingReadLine(QString* string, int msec, char lineEndChar)
{
	QByteArray line;
	bool result = blockingReadLine( &line, msec, lineEndChar);
	if (Settings::IS_WINDOWS)
	{
		*string = QString::fromUtf8(line);
	}
	else if ( Settings::IS_MAC )
	{
		QString inputString = QString::fromUtf8(line);
		*string = inputString.normalized(QString::NormalizationForm_C);
	}
	else
	{
		*string = QString::fromLocal8Bit(line);
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
	qDebug() << "Process::terminate()";
	if (qProcess)
	{
		qProcess->kill();
		int n = 0;
		while (++n < 20 && qProcess->state() == QProcess::Running) {
			qProcess->waitForFinished(500);
			qDebug() << "waiting...";
		}
		delete qProcess;
		qProcess = 0;
	}
}

