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

#include <Qt>
#include <QDebug>
#include <QSet>
#include <QFileInfo>

#include "model/local_dir_model.hh"

LocalDirModel::LocalDirModel(const QStringList & nameFilters, QDir::Filters filters, QDir::SortFlags sort, QObject * parent) : QDirModel(nameFilters, filters, sort, parent)
{
	selectionRules.clear();
}

Qt::ItemFlags LocalDirModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = QDirModel::flags(index);
	if (index.column() == 0) // make the first column checkable
		f |= Qt::ItemIsUserCheckable;
	return f;
}
 
/**
 * closest rule on parent directories -> subState
 * if any of the childs
 */
QVariant LocalDirModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
	{
		QFileInfo f_info = fileInfo(index);
		QString curPath = f_info.absoluteFilePath() + (f_info.isDir()?"/":"");
		bool existRulesOnChildren = false;
		QPair<QString,bool> closestParentRule;
		QHashIterator<QString,bool> i(selectionRules);
		while (i.hasNext()) {
			i.next();
			if (i.key() != curPath && i.key().startsWith(curPath)) {
				// rules on children
				existRulesOnChildren = true;
			} else if (curPath.startsWith(i.key())) {
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
	return QDirModel::data(index, role);
}
/**
 * remove all rules on children
 * if rule exists on current node: negate it
 * else: add rule: (cur==checked)?excl:incl
 */
bool LocalDirModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
	{
		Qt::CheckState curVal = (Qt::CheckState)(data(index,role).toInt());
		QFileInfo f_info = fileInfo(index); 
		bool isDir = f_info.isDir();
		QString curPath = f_info.absoluteFilePath() + (isDir?"/":"");
		QPair<QString,bool> closestParentRule("",false);
		QMutableHashIterator<QString,bool> i(selectionRules);
		while (i.hasNext()) {
			i.next();
			if (i.key().startsWith(curPath)) {
				// rules on children and self
				i.remove();
			} else if (curPath.startsWith(i.key())) {
				// rules on parents
				if (i.key().length() > closestParentRule.first.length()) {
					// rule is "closer" -> take it
					closestParentRule = QPair<QString,bool>(i.key(),i.value());
				}
			}
		}
		if ((curVal == Qt::Checked)==closestParentRule.second) {
			selectionRules.insert(curPath, !closestParentRule.second);
		}
		emit layoutChanged(); // in fact dataChanged() would be better, but it's too expensive to calculate all the changed items
		return true;
	}
	return QDirModel::setData(index, value, role);
}


LocalDirModel::~LocalDirModel()
{
}

