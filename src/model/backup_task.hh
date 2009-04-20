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



class BackupTask
{
	public:
		enum StatusEnum
		{ OK, WARNING, ERROR, UNDEFINED };
		BackupTask();
		BackupTask( const BackupTask& newBackupTask );
		BackupTask( const QDateTime& backupTime, BackupTask::StatusEnum status );
		~BackupTask();

		static QString getStatusText( BackupTask::StatusEnum status );
		QString getStatusText() const;
		StatusEnum getStatus() const;
		void setStatus( const StatusEnum& status );
		QDateTime getDateTime() const;
		void setDateTime( const QDateTime& backupTime );
		QString toString() const;
		bool equals( const BackupTask& bTask ) const;
		bool equalDateTime( const BackupTask& bTask ) const;

	private:
		QDateTime backupTime;
		StatusEnum status;
		static QMap<StatusEnum, QString> map_statusText;
};

#endif /*BACKUPTASK_H_*/
