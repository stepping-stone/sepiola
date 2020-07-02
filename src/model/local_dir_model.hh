/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
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

#ifndef LOCAL_DIR_MODEL_HH
#define LOCAL_DIR_MODEL_HH

#include <QDir>
#include <QFileSystemModel>
#include <QSet>
#include <QStringList>

#include "utils/datatypes.hh"

/**
 * The LocalDirModel class represents a local file system
 * @author Dominic Sydler, sydler@puzzle.ch
 * @source
 * http://www.qtcentre.org/forum/f-qt-programming-2/t-qdirmodelqtreeview-and-checkable-items-6957.html
 */

class LocalDirModel : public QFileSystemModel
{
public:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    const BackupSelectionHash &getSelectionRules() { return selectionRules; };
    void setSelectionRules(const BackupSelectionHash &selectionRules)
    {
        this->selectionRules = selectionRules;
    };

private:
    BackupSelectionHash selectionRules;
};

#endif /* LOCAL_DIR_MODEL_HH */
