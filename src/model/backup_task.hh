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

#ifndef BACKUPTASK_H_
#define BACKUPTASK_H_

#include <QList>
#include <QStringList>
#include <QDebug>

#include "tools/abstract_rsync.hh"
#include "exception/abort_exception.hh"
#include "utils/const_utils.hh"



class BackupTask
{
	public:
		BackupTask();
		BackupTask( const BackupTask& newBackupTask );
		BackupTask( const QDateTime& backupTime, ConstUtils::StatusEnum status );
		~BackupTask();

		static QString getStatusText( ConstUtils::StatusEnum status );
		QString getStatusText() const;
		ConstUtils::StatusEnum getStatus() const;
		void setStatus( const ConstUtils::StatusEnum& status );
		QDateTime getDateTime() const;
		void setDateTime( const QDateTime& backupTime );
		QString toString() const;
		bool equals( const BackupTask& bTask ) const;
		bool equalDateTime( const BackupTask& bTask ) const;

	private:
		QDateTime backupTime;
		ConstUtils::StatusEnum status;
		static QMap<ConstUtils::StatusEnum, QString> map_statusText;
};

#endif /*BACKUPTASK_H_*/
