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

#ifndef FILE_SYSTEM_UTILS_HH
#define FILE_SYSTEM_UTILS_HH

#include "exception/runtime_exception.hh"

#include <QFileInfo>
#include <QString>

/**
 * The FileSystemUtils class provides convenience methods for file system specific tasks
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class FileSystemUtils
{
private:
	FileSystemUtils();
	virtual ~FileSystemUtils();

public:

	/**
	 * Removes a directory. If the recursive flag is set to true, it will
	 * remove the directory even if it's not empty
	 */
	static bool rmDirRecursive( const QString& directory );

	/**
	 * Delete's a given file
	 * @param fileInfo file to delete
	 */
	static void removeFile( const QFileInfo& fileInfo );

	/**
	 * Converts \a file \a fromEncoding \a toEncoding.
	 */
	static void convertFile(const QString& file, const QString& fromEncoding, const QString& toEncoding);

	/**
	 * Converts the given path so that rsync supports it.
	 * If the application's operating system is a Unix (Linux and Mac)
	 * no conversation will be made.
	 * If the application is running on a Microsoft Windows, the following
	 * conversion happens: the drive will be changed to a folder. For instance,
	 * if the given path is c:\\folder\\file, the path will be changed to /c/folder/file
	 * @param path the path to convert
	 */
	static void convertToServerPath( QString* path );

    /**
     * Converts the given path so that cygwin supports it.
     * If the application's operating system is a Unix (Linux and Mac)
     * no conversation will be made.
     * If the application is running on a Microsoft Windows, the following
     * conversion happens: the drive will be changed to a folder. For instance,
     * if the given path is C/folder/file, the path will be changed to /cygdrive/c/folder/file
     * @param path the path to convert
     */
    static void convertToCygwinPath(QString* path);

	/**
	 * Converts the given path so that the underlaying operating system supports it.
	 * If the application's operating system is a Unix (Linux and Mac)
	 * no conversation will be made.
	 * If the application is running on a Microsoft Windows, the following
	 * conversion happens: the first folder will be changed to a drive. For instance,
	 * if the given path is /c/folder/file, the path will be changed to c:\\folder\\file
	 * @param path the path to convert
	 */
	static void convertToLocalPath( QString* path );

	/**
	 * Checks if this path points to a root directory. The root directory is "/" on Linux and Mac,
	 * on Windows it is a letter followed by a ":", for instance c:
	 * @param path path to check
	 * @return true if path is a root directory
	 */
	static bool isRoot( const QString& path );

    /**
     * Returns the root directory on the corresponding operating system. The root directory is "/" on Linux and Mac,
     * on Windows it is a letter followed by a ":" and a "\", for instance C:\
     * @param path: Path to determine the root directory.
     * @return param: A string, which contains the root directory. "/" on Linux and Mac, "C:\" on Windows.
     */
    static QString getRootItemFromAbsPath( const QString& path);

    /**
     * The filename will be something like <LETTER>:\path\to\file. This function returns the <LETTER>
     * @param filename: e.g. C:\path\to\file
     * @return The drive letter: <LETTER> e.g. C
     */
    static QString getDriveLetterByFile( const QString filename );

	/**
	 * Checks if this path points to a directory
	 * @param path path to check
	 * @return true if path is a directory
	 */
	static bool isDir( const QString& path );

	/**
	 * Writes the given lines to a file. If the file already exists, the
	 * @param fileName absolute file path
	 * @param lines lines to write to the file
	 */
	static void writeLinesToFile( const QString& fileName, const QStringList& lines, const QString& encoding = "" );

	/**
	 * Gets a list of all lines of a file
	 * @param fileName absolute file path
	 * @return list of lines in file
	 */
	static QStringList readLinesFromFile( const QString& fileName, const QString& encoding = "" );

};
#endif
