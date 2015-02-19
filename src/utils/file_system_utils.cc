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

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTextStream>

#include "utils/file_system_utils.hh"
#include "test/test_manager.hh"
#include "settings/platform.hh"
#include "exception/runtime_exception.hh"

FileSystemUtils::FileSystemUtils()
{
}

FileSystemUtils::~FileSystemUtils()
{
}

bool FileSystemUtils::rmDirRecursive(const QString& directory)
{
	QString directoryName = directory;
	if ( !directoryName.endsWith("/") )
	{
		directoryName.append("/");
	}
	QDir dir(directoryName);
	QList<QFileInfo> dirEntries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
	foreach( QFileInfo entry, dirEntries )
	{
		if ( entry.isDir() )
		{
			rmDirRecursive( entry.absoluteFilePath() ); // recursive call for deleting directory
		}
		else
		{
			QFile::remove( entry.absoluteFilePath() ); // delete file
		}
	}
	return dir.rmdir(directoryName);
}

void FileSystemUtils::removeFile(const QFileInfo& fileInfo)
{
	QFile file(fileInfo.absoluteFilePath() );
	file.remove();
}

void FileSystemUtils::convertFile(const QString& fileName, const QString& fromEncoding, const QString& toEncoding)
{
	qDebug() << "FileSystemUtils::convertFile" << " " << fileName << " " << fromEncoding << " " << toEncoding;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
	    throw RuntimeException( QObject::tr( "Could not open file %1" ).arg(fileName) );
	qDebug() << "File to convert: " << fileName << " size: " << file.size();
	QTemporaryFile tmpFile(fileName);
	tmpFile.setAutoRemove(false);
	if (!tmpFile.open())
	    throw RuntimeException( QObject::tr( "Could not open temporary file %1" ).arg(tmpFile.fileName()) );

	qDebug() << "tmpFile: " << tmpFile.fileName();

	QTextStream inputStream(&file);
	if (!fromEncoding.isEmpty())
		inputStream.setCodec(fromEncoding.toLatin1());

	QTextStream outputStream(&tmpFile);
	if (!toEncoding.isEmpty())
		outputStream.setCodec(toEncoding.toLatin1());

	while (!inputStream.atEnd())
	{
		outputStream << inputStream.readLine() << endl;
		if (tmpFile.error() != 0)
		{
			throw RuntimeException( QObject::tr( "converted data could not be written to temporary file %1" ).arg(tmpFile.fileName()) );
		}
	}
	outputStream.flush();
	file.close();
	tmpFile.close();
	// overwrite file by tmpFile
	if (!file.remove())
		throw RuntimeException( QObject::tr( "Trying to rename file %1 to %2:\n  -> file %1 could nod be removed." ).arg(tmpFile.fileName(), file.fileName()) );
	if (!tmpFile.rename(fileName))
		throw RuntimeException( QObject::tr( "Could not rename file %1 to %2." ).arg(tmpFile.fileName(), file.fileName()) );
}

#ifdef Q_OS_WIN32
void FileSystemUtils::convertToServerPath(QString* path)
{
    *path = QDir::fromNativeSeparators( *path);
    if (path->size()> 2 && (*path)[0].isLetter() && (*path)[1] == ':' && (*path)[2] == '/')
    {
        *path = QString("/") + (*path)[0].toUpper() + path->right(path->size() - 2);
    }
}
#else
void FileSystemUtils::convertToServerPath(QString*)
{
}
#endif

#ifdef Q_OS_WIN32
void FileSystemUtils::convertToLocalPath(QString* path)
{
    if (path->size()> 2 && (*path)[0] == '/' && (*path)[1].isLetter() && (*path)[2] == '/')
    {
        *path = (*path)[1] + ":" + path->right(path->size() - 2);
        *path = QDir::toNativeSeparators( *path);
        return;
    }
    if (path->size()> 1 && (*path)[0].isLetter() && (*path)[1] == '/')
    {
        *path = (*path)[0] + ":" + path->right(path->size() - 1);
        *path = QDir::toNativeSeparators( *path);
        return;
    }
}
#else
void FileSystemUtils::convertToLocalPath(QString*)
{
}
#endif

bool FileSystemUtils::isRoot(const QString& path)
{
	return ( (path.size() == 1 && path.startsWith( '/') ) || (path.size() == 3 && path[0].isLetter() && path[1] == ':' ) );
}

bool FileSystemUtils::isDir(const QString& path)
{
	return path.endsWith("/") || path.endsWith("\\");
}

void FileSystemUtils::writeLinesToFile(const QString& fileName, const QStringList& lines, const QString& encoding)
{
	qDebug() << "FileSystemUtils::writeLinesToFile( " << fileName << ", ... )";
	QStringList result;
	QFile file(fileName);
	if ( !file.open(QIODevice::WriteOnly) )
	{
		qWarning() << "Can not write to file " + fileName;
	}
	QTextStream out( &file);
	if (!encoding.isEmpty())
	{
		out.setCodec(encoding.toLatin1());
		//out.setGenerateByteOrderMark(true);
	}
	foreach( QString line, lines )
	{
		out << line << Platform::EOL_CHARACTER;
	}
}

QStringList FileSystemUtils::readLinesFromFile(const QString& fileName, const QString& encoding)
{
	qDebug() << "FileSystemUtils::readLinesFromFile( " << fileName << " )";
	QStringList result;
	QFile file(fileName);
	if ( !file.open(QIODevice::ReadWrite) )
	{
		qWarning() << "Can not read from file " + fileName;
		return result;
	}
	QTextStream in(&file);
	if (!encoding.isEmpty())
	{
		in.setCodec(encoding.toLatin1());
	}
	while ( !in.atEnd() )
	{
		QString line = in.readLine();
		result << line;
	}
	return result;
}
