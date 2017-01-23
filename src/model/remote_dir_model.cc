/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2011 stepping stone GmbH
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
    qSort(backupContent.begin(), backupContent.end(), fileLessThan);
    if (!backupContent.isEmpty() && !FileSystemUtils::isRoot(backupContent.first()))
    {
        backupContent.push_front( FileSystemUtils::getRootItemFromAbsPath(backupContent.first()));
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

Qt::ItemFlags RemoteDirModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = QStandardItemModel::flags(index);
	if (index.column() == 0) // make the first column checkable
		f |= Qt::ItemIsUserCheckable;
	return f;
}

/**
 * closest rule on parent directories -> subState
 * if any of the childs
 */
QVariant RemoteDirModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
	{
		QFileInfo f_info = QFileInfo( ((DirTreeItem*)this->itemFromIndex( index ))->getAbsoluteName() );
		bool curPathIsDir = f_info.absoluteFilePath().endsWith("/"); // no other possibility found at the moment
		QString curPath = f_info.absoluteFilePath() + ((curPathIsDir &&  !f_info.absoluteFilePath().endsWith("/"))?"/":"");
		bool existRulesOnChildren = false;
		QPair<QString,bool> closestParentRule;
		QHashIterator<QString,bool> i(selectionRules);
		while (i.hasNext()) {
			i.next();
			if (i.key() != curPath && curPathIsDir && i.key().startsWith(curPath)) {
				// rules on children
				existRulesOnChildren = true;
			} else if (i.key() == curPath || (i.key().endsWith("/") && curPath.startsWith(i.key()))) {
				// rules on parents or self
				if (i.key().length() > closestParentRule.first.length()) {
					// rule is "closer" -> take it
					closestParentRule = QPair<QString,bool>(i.key(),i.value());
				}
			}
		}
        // the item is checked only if we have stored its path
		return (existRulesOnChildren ? 0 : 1)*((closestParentRule.second ? Qt::Checked : Qt::Unchecked)-1) + 1;
	}
	return QStandardItemModel::data(index, role);
}
/**
 * remove all rules on children
 * if rule exists on current node: negate it
 * else: add rule: (cur==checked)?excl:incl
 */
bool RemoteDirModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
	{
		Qt::CheckState curVal = (Qt::CheckState)(data(index,role).toInt());
		QFileInfo f_info = QFileInfo( ((DirTreeItem*)this->itemFromIndex( index ))->getAbsoluteName() );
		bool curPathIsDir = f_info.absoluteFilePath().endsWith("/"); // no other possibility found at the moment
		QString curPath = f_info.absoluteFilePath() + ((curPathIsDir &&  !f_info.absoluteFilePath().endsWith("/"))?"/":"");
		// qDebug() << "RemoteDirModel::setData(" << curPath << ")";
		QPair<QString,bool> closestParentRule("",false);
		QMutableHashIterator<QString,bool> itr(selectionRules);
		while (itr.hasNext()) {
			itr.next();
			QString rulePath = itr.key();
			bool rulePathIsDir = rulePath.endsWith("/");
			if (rulePath == curPath || (curPathIsDir && rulePath.startsWith(curPath))) {
				// rules on children and self
				itr.remove();
			} else if (rulePathIsDir && curPath.startsWith(rulePath)) {
				// rules on parents
				if (rulePath.length() > closestParentRule.first.length()) {
					// rule is "closer" -> take it
					closestParentRule = QPair<QString,bool>(rulePath, itr.value());
				}
			}
		}
		if ((curVal == Qt::Checked)==closestParentRule.second) {
			selectionRules.insert(curPath, !closestParentRule.second);
		}
		emit layoutChanged(); // in fact dataChanged() would be better, but it's too expensive to calculate all the changed items
		return true;
	}
	return QStandardItemModel::setData(index, value, role);
}

RemoteDirModel::~RemoteDirModel()
{
}

