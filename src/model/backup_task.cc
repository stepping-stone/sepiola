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
#include <QDateTime>

#include "model/backup_task.hh"

QMap<BackupTask::StatusEnum, QString> BackupTask::map_statusText;


BackupTask::BackupTask()
{
	this->setDateTime( QDateTime() );
	this->setStatus( BackupTask::UNDEFINED );
}

BackupTask::BackupTask( const BackupTask& newBackupTask )
{
	backupTime = newBackupTask.backupTime;
	status = newBackupTask.status;
}

BackupTask::BackupTask( const QDateTime& backupTime, BackupTask::StatusEnum status )
{
	BackupTask();
	this->setStatus( status );
	this->setDateTime( backupTime );
}

BackupTask::~BackupTask() {}

QDateTime BackupTask::getDateTime() const
{
	return this->backupTime;
}

void BackupTask::setDateTime( const QDateTime& backupTime )
{
	this->backupTime = backupTime;
}

QString BackupTask::getStatusText( BackupTask::StatusEnum status )
{
	if ( map_statusText.size() == 0 )
	{
		map_statusText.insert( OK, QObject::tr( "<font color=\"green\">Successful</font>" ) );
		map_statusText.insert( WARNING, QObject::tr( "<font color=\"orange\">Warnings</font>" ) );
		map_statusText.insert( ERROR, QObject::tr( "<font color=\"red\">Failed</font>" ) );
		map_statusText.insert( UNDEFINED, QObject::tr( "undefined" ) );
	}
	return map_statusText.value( status );
}
QString BackupTask::getStatusText() const
{
	return getStatusText( this->status );
}
BackupTask::StatusEnum BackupTask::getStatus() const
{
	return( this->status );
}
void BackupTask::setStatus( const BackupTask::StatusEnum& status )
{
	this->status = status;
}

QString BackupTask::toString() const
{
	return QString( "%1 on %2" ).arg( getStatusText(), getDateTime().toString() );
}

bool BackupTask::equals( const BackupTask& bTask ) const
{
	return ( this->backupTime == bTask.backupTime && this->status == bTask.status );
}

bool BackupTask::equalDateTime( const BackupTask& bTask ) const
{
	return ( this->backupTime == bTask.backupTime );
}