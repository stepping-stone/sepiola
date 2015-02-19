/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2015 stepping stone GmbH
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

#include "model/backup_task.hh"

QMap<ConstUtils::StatusEnum, QString> BackupTask::map_statusText;

BackupTask::BackupTask() :
	backupTime(QDateTime()),
	status(ConstUtils::STATUS_UNDEFINED)
{
}

BackupTask::BackupTask( const BackupTask& newBackupTask )
{
	backupTime = newBackupTask.backupTime;
	status = newBackupTask.status;
}

BackupTask::BackupTask( const QDateTime& bT, ConstUtils::StatusEnum s ) :
	backupTime(bT),
	status(s)
{
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

QString BackupTask::getStatusText( ConstUtils::StatusEnum status )
{
	if ( map_statusText.size() == 0 )
	{
		map_statusText.insert( ConstUtils::STATUS_OK, QObject::tr( "<font color=\"green\">Successful</font>" ) );
		map_statusText.insert( ConstUtils::STATUS_WARNING, QObject::tr( "<font color=\"orange\">Warnings</font>" ) );
		map_statusText.insert( ConstUtils::STATUS_ERROR, QObject::tr( "<font color=\"red\">Failed</font>" ) );
		map_statusText.insert( ConstUtils::STATUS_UNDEFINED, QObject::tr( "undefined" ) );
	}
	return map_statusText.value( status );
}
QString BackupTask::getStatusText() const
{
	return getStatusText( this->status );
}
ConstUtils::StatusEnum BackupTask::getStatus() const
{
	return( this->status );
}
void BackupTask::setStatus( const ConstUtils::StatusEnum& status )
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
