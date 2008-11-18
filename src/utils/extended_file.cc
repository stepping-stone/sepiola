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

#include <sys/stat.h>
#include "utils/extended_file.hh"

QCache<QString, uint> ExtendedFile::_ownerIdCache(10000);
QCache<QString, uint> ExtendedFile::_groupIdCache(10000);
QCache<uint, QString> ExtendedFile::_ownerCache(10000);
QCache<uint, QString> ExtendedFile::_groupCache(10000);
QCache<QString, FileInfo> ExtendedFile::_fileInfoCache(10000);

void ExtendedFile::clearCaches()
{
	_ownerIdCache.clear();
	_groupIdCache.clear();
	_ownerCache.clear();
	_groupCache.clear();
	_fileInfoCache.clear();
}

#ifdef Q_OS_UNIX
FileInfo ExtendedFile::fileInfo() const
{
	FileInfo* fileInfo = _fileInfoCache[fileName()];
	if ( !fileInfo )
	{
		struct stat statBuf;
		if (stat( encodeName( fileName() ), &statBuf ) == 0 )
		{
			fileInfo = new FileInfo(statBuf.st_uid, statBuf.st_gid);
		}
		else
		{
			fileInfo = new FileInfo;
		}
		_fileInfoCache.insert( fileName(), fileInfo );
	}

	return *fileInfo;
}

uint ExtendedFile::ownerToId( const QString& owner )
{
	uint* ownerId = _ownerIdCache[owner];
	if ( !ownerId )
	{
		struct passwd* pwent = getpwnam( owner.toLocal8Bit() );
		if (pwent)
		{
			ownerId = new uint( pwent->pw_uid );
		}
		else
		{
			ownerId = new uint( -1 );
		}
		_ownerIdCache.insert( owner, ownerId );
	}
	return *ownerId;
}

uint ExtendedFile::groupToId( const QString& group )
{
	uint* groupId = _groupIdCache[group];
	if ( !groupId )
	{
		struct group* grent = getgrnam( group.toLocal8Bit() );
		if (grent)
		{
			groupId = new uint( grent->gr_gid );
		}
		else
		{
			groupId = new uint( -1 );
		}
		_groupIdCache.insert( group, groupId );
	}
	return *groupId;
}

QString ExtendedFile::idToOwner( uint uid )
{
	QString* owner = _ownerCache[uid];
	if ( !owner )
	{
		struct passwd* pwent = getpwuid( uid );
		if (pwent)
		{
			owner = new QString( pwent->pw_name );
		}
		else
		{
			owner = new QString;
		}
		_ownerCache.insert( uid, owner );
	}
	return *owner;
}

QString ExtendedFile::idToGroup( uint gid )
{
	QString* group = _groupCache[gid];
	if ( !group )
	{
		struct group* grent = getgrgid( gid );
		if (grent)
		{
			group = new QString( grent->gr_name );
		}
		else
		{
			group = new QString;
		}
		_groupCache.insert( gid, group );
	}
	return *group;
}
#else
FileInfo ExtendedFile::fileInfo() const
{
	return FileInfo();
}

uint ExtendedFile::ownerToId( const QString& owner )
{
	return uint(-1);
}

uint ExtendedFile::groupToId( const QString& group )
{
	return uint(-1);
}

QString ExtendedFile::idToOwner( uint uid )
{
	return QString();
}

QString ExtendedFile::idToGroup( uint gid )
{
	return QString();
}
#endif
