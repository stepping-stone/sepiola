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

#ifndef DIR_TREE_ITEM_HH
#define DIR_TREE_ITEM_HH

#include <QStandardItem>
#include <QFileIconProvider>

/**
 * The DirTreeItem class represents an item (file or directory) for the RemoteDirModel
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class DirTreeItem : public QStandardItem
{
public:
	/**
	 * Constructs a DirTreeItem representing a path
	 * @param path a path
	 */
	DirTreeItem( const QString& path, const QFileIconProvider& iconProvider );

	/**
	 * Destroys the DirTreeItem
	 */
	virtual ~DirTreeItem();

	/**
	 * Gets the absolute file or directory name
	 * @return the absolute path
	 */
	QString getAbsoluteName();

private:
	QString absoluteName;
};

inline QString DirTreeItem::getAbsoluteName()
{
	return this->absoluteName;
}

#endif

