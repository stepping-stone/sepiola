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

#include <Qt>
#include <QDebug>
#include <QSet>
#include <QFileInfo>

#include "model/local_dir_model.hh"

Qt::ItemFlags LocalDirModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = QFileSystemModel::flags(index);
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
		QFileInfo f_info  = fileInfo(index);
		bool curPathIsDir = f_info.isDir();
		QString curPath   = f_info.absoluteFilePath();

		if (curPathIsDir &&  !curPath.endsWith("/"))
			curPath += "/";

		bool existRulesOnChildren = false;
		QPair<QString,bool> closestParentRule;

		QHashIterator<QString,bool> itr(selectionRules);
		while (itr.hasNext()) {
			itr.next();

			QString rulePath = itr.key();
			if (rulePath != curPath && curPathIsDir && rulePath.startsWith(curPath)) {
				// rules on children
				existRulesOnChildren = true;
			} else if (rulePath == curPath || (rulePath.endsWith("/") && curPath.startsWith(rulePath))) {
				// rules on parents or self
				if (rulePath.length() > closestParentRule.first.length()) {
					// rule is "closer" -> take it
					closestParentRule = QPair<QString,bool>(rulePath, itr.value());
				}
			}
		}

		if (existRulesOnChildren)
			return Qt::PartiallyChecked;

		if (closestParentRule.second)
			return Qt::Checked;

		return Qt::Unchecked;
	}

	return QFileSystemModel::data(index, role);
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
		bool curPathIsDir = f_info.isDir();
		QString curPath = f_info.absoluteFilePath();

		if (curPathIsDir &&  !curPath.endsWith("/"))
			curPath += "/";

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

		if ((curVal == Qt::Checked) == closestParentRule.second) {
			selectionRules.insert(curPath, !closestParentRule.second);
		}
		emit layoutChanged(); // in fact dataChanged() would be better, but it's too expensive to calculate all the changed items

		return true;
	}

	return QFileSystemModel::setData(index, value, role);
}
