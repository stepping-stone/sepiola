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

#ifndef REMOTE_DIR_MODEL_HH
#define REMOTE_DIR_MODEL_HH

#include <QStandardItem>
#include <QStringList>
#include <QFileInfo>

/**
 * The RemoteDirModel class represents a remote file system
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: bsantschi $ $Date: 2008/04/25 05:42:15 $ $Revision: 1.14 $
 */
class RemoteDirModel : public QStandardItemModel
{
public:

	/**
	 * Constructs a RemoteDirModel with the given items
	 * @param backupContent list of files and directories
	 */
	RemoteDirModel( QStringList backupContent );

	/**
	 * Destroys the RemoteDirModel
	 */
	virtual ~RemoteDirModel();
};

QString sortKey(QString file);

inline bool fileLessThan(const QString& file1, const QString& file2)
{
	return QString::localeAwareCompare(sortKey(file1), sortKey(file2)) < 0;
}

#endif

