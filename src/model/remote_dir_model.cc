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

#include <QStack>
#include <QDir>
#include <QDebug>

#include "model/remote_dir_model.hh"
#include "model/dir_tree_item.hh"
#include "settings/settings.hh"
#include "utils/file_system_utils.hh"

#include <iostream>

QString sortKey(QString file)
{
	file = QDir::fromNativeSeparators(file);
	file.replace('/', "/0");
	if (!file.endsWith("/0"))
	{
		int index = file.lastIndexOf("/0");
		if (index > 0 ) file[index + 1] = '1';
	}

	return file;
}

RemoteDirModel::RemoteDirModel( QStringList backupContent )
{
	qDebug() << "RemoteDirModel::RemoteDirModel(QStringList)";

	qSort(backupContent.begin(), backupContent.end(), fileLessThan);

	if (!backupContent.isEmpty() && !FileSystemUtils::isRoot(backupContent.first()))
	{
		backupContent.push_front("/");
	}

	QFileIconProvider iconProvider;
	setHorizontalHeaderLabels( QStringList( QObject::tr( "Name" ) ) );
	QStack< QPair< QString, QStandardItem* > > dirStack;
	dirStack.push( qMakePair( QString(), invisibleRootItem() ) );
	foreach ( QString file, backupContent )
	{
		if ( file.isEmpty() ) continue;
		while ( dirStack.size() > 1 && !file.startsWith( dirStack.top().first ) )
		{
			dirStack.pop();
		}
		if ( FileSystemUtils::isDir( file ) )
		{
			QStandardItem* dirTreeItem = new DirTreeItem( file, iconProvider );
			dirStack.top().second->appendRow( dirTreeItem );
			dirStack.push( qMakePair( file, dirTreeItem ) );
		}
		else
		{
			dirStack.top().second->appendRow( new DirTreeItem( file, iconProvider ) );
		}
	}

	qDebug() << "RemoteDirModel::RemoteDirModel(QStringList) done";
}

RemoteDirModel::~RemoteDirModel()
{
}

