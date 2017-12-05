/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2017 stepping stone GmbH
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

#ifndef EXTENDED_FILE_HH
#define EXTENDED_FILE_HH

#include <QCache>
#include <QFile>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#endif

/**
 * The FileInfo class is used to hold a pair of owner and group identifier
 * @author Daniel Tschan, tschan@puzzle.ch
 */
class FileInfo
{
public:
	/**
	 * Constructs a FileInfo
	 * @param uid identifier of the owner
	 * @param gid identifier of the group
	 */
	FileInfo(uint uid = -1, uint gid = -1) : ownerId(uid), groupId(gid) {}

	/**
	 * Identifier of the owner
	 */
	uint ownerId;

	/**
	 * Identifier of the group
	 */
	uint groupId;
};

/**
 * The ExtendedFile class is used to set the owner and the group for a file or directory
 * @author Daniel Tschan, tschan@puzzle.ch
 */
class ExtendedFile : public QFile
{
public:

	/**
	 * Constructs an ExtendedFile for the given file name
	 * @param file name of the file
	 */
	ExtendedFile(const QString& file);

	/**
	 * Destroys the ExtendedFile
	 */
	~ExtendedFile();

	/**
	 * Sets the owner and group based on their identifiers
	 * @param ownerId ID of the owner
	 * @param groupId ID of the group
	 * @return true if owner and group has been set
	 */
	bool setOwnerAndGroupId(uint ownerId, uint groupId);

	/**
	 * Sets the owner and group based on their names
	 * @param owner name of the owner
	 * @param group name of the group
	 * @return true if owner and group has been set
	 */
	bool setOwnerAndGroup(const QString& owner, const QString& group);

	/**
	 * Gets the owner of this file
	 * @return the owner of this file
	 */
	QString owner() const;

	/**
	 * Gets the group of this file
	 * @return the owner of this file
	 */
	QString group() const;

	/**
	 * Identifier of the owner
	 */
	uint ownerId() const;

	/**
	 * Identifier of the group
	 */
	uint groupId() const;

	/**
	 * Clears all cached items
	 */
	static void clearCaches();

private:
	static uint ownerToId(const QString& owner);
	static uint groupToId(const QString& group);
	static QString idToOwner(uint uid);
	static QString idToGroup(uint gid);
	FileInfo fileInfo() const;

private:
	static QCache<QString, uint> _ownerIdCache;
	static QCache<QString, uint> _groupIdCache;
	static QCache<uint, QString> _ownerCache;
	static QCache<uint, QString> _groupCache;
	static QCache<QString, FileInfo> _fileInfoCache;
};

inline ExtendedFile::ExtendedFile(const QString& file) : QFile(file)
{
}

inline ExtendedFile::~ExtendedFile()
{
}

inline bool ExtendedFile::setOwnerAndGroup(const QString& owner, const QString& group)
{
	return setOwnerAndGroupId(ownerToId(owner), groupToId(group));
}

inline QString ExtendedFile::owner() const
{
	return idToOwner( ownerId() );
}

inline QString ExtendedFile::group() const
{
	return idToGroup( groupId() );
}

inline uint ExtendedFile::ownerId() const
{
	return fileInfo().ownerId;
}

inline uint ExtendedFile::groupId() const
{
	return fileInfo().groupId;
}

#ifdef Q_OS_UNIX
inline bool ExtendedFile::setOwnerAndGroupId(uint ownerId, uint groupId)
{
	return chown(encodeName( fileName() ), ownerId, groupId) == 0;
}
#else
inline bool ExtendedFile::setOwnerAndGroupId(uint, uint)
{
	return true;
}
#endif

#endif
