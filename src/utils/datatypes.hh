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

#ifndef DATATYPES_HH
#define DATATYPES_HH

#include <QMetaType>
#include <QHash>
#include <QString>
#include <QPair>
#include <QList>

typedef QList<QPair<QString,QString> > StringPairList;
Q_DECLARE_METATYPE( StringPairList )
typedef QHash<QString,bool> BackupSelectionHash;
Q_DECLARE_METATYPE( BackupSelectionHash )

#endif /*DATATYPES_HH*/
