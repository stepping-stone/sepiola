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

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFileIconProvider>

#include "dir_tree_item.hh"
#include "utils/file_system_utils.hh"

DirTreeItem::DirTreeItem( const QString& path, const QFileIconProvider& iconProvider )
{
	setEditable(false);
	this->setCheckable(true);
	this->setCheckState(Qt::Unchecked);
			
	this->absoluteName = path;
	QFileInfo fileInfo( path );

	if ( FileSystemUtils::isDir( path ) )
	{
		if ( FileSystemUtils::isRoot( path ) )
		{
			setIcon( iconProvider.icon( QFileIconProvider::Drive ) );
			setText( path );
		}
		else
		{
			setIcon( iconProvider.icon( QFileIconProvider::Folder ) );
			setText( fileInfo.dir().dirName() );
		}
	}
	else
	{
		setIcon( iconProvider.icon( QFileIconProvider::File ) );
		setText( fileInfo.fileName() );
	}
}

DirTreeItem::~DirTreeItem()
{
}

