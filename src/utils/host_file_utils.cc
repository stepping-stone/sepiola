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

#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "utils/host_file_utils.hh"
#include "utils/file_system_utils.hh"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QtEndian>
#include <QHostInfo>

HostFileUtils::HostFileUtils()
{
}
HostFileUtils::~HostFileUtils()
{
}

void HostFileUtils::addPuttyKeyToOpenSshKeyFile()
{
	QString host = Settings::getInstance()->getServerName();
	QString puttyKey = getPuttyKey( host );
	if( puttyKey == "" )
	{
		qDebug() << "no putty key found for host " << host;
	}
	else
	{
		QString sshKey = convertPuttyKey( puttyKey, host );
		addOpenSshKey( sshKey, host );
	}
}

QString HostFileUtils::getPuttyKey( const QString& host )
{
	QString sshhostkeysFileName = QDir::home().absolutePath() + "/.putty/sshhostkeys"; //TODO: move string to config
	QStringList allKeys = FileSystemUtils::readLinesFromFile( sshhostkeysFileName );

		QString hostSearchPattern = ":" + host + " "; //rsa2@22:host 0x23,0x
		foreach( QString key, allKeys )
		{
			if( key.contains( hostSearchPattern )) {
				return key;
			}
		}
		qDebug() << "key not found";
		return "";
}

QString HostFileUtils::convertPuttyKey( const QString& puttyKeyString, const QString& host )
{
	QByteArray puttyKey;
	int keyStartPosition = 16 + host.length(); //rsa2@22:host 0x23,0x
	puttyKey.append( puttyKeyString.mid( keyStartPosition ) );

	// key example: 00 00 00 07  73 73 68 2D  72 73 61 00  00 00 01 23  00 00 00 81  00 9F 5F 3C  AB 99 A2 D
	QByteArray entry = QByteArray::fromHex( "00000007" );
	entry.append("ssh-rsa");
	entry.append( QByteArray::fromHex( "0000000123" ) );

	// key length
	qDebug() << QByteArray::fromHex( puttyKey ).length();
	int length = qToBigEndian( QByteArray::fromHex( puttyKey ).length() + 1 );
	//TODO:  mit right die rechten 4 bytes der Länge nehmen.
	// Auf 32 und 64 pcs ist int 32 bt breit -> auf Mac tetsten
	QByteArray lengthBytes((const char*)&length, sizeof(length));
	entry.append( lengthBytes );

	entry.append( QByteArray::fromHex( "00" ) );

	// putty key
	entry.append( QByteArray::fromHex( puttyKey ) );
	QString result;
	result.append( host );
	result.append( "," );
	result.append( getIpAddress( host ) );
	result.append( " ssh-rsa ");
	result.append( entry.toBase64() );
	return result;
}

void HostFileUtils::addOpenSshKey( const QString& openSshKey, const QString& host )
{
	QString knownHostsFile = QDir::home().absolutePath() + "/.ssh/known_hosts"; //TODO: move string to config
	QStringList oldKeys = FileSystemUtils::readLinesFromFile( knownHostsFile );
	QStringList newKeys;

		QString hostSearchPattern = host + ",";
		bool added = false;
		foreach( QString key, oldKeys )
		{
			if( key.startsWith( hostSearchPattern ) )
			{
				// replace the key
				newKeys << openSshKey;
				added = true;
			}
			else
			{
				newKeys << key;
			}
		}
		if( !added )
		{
			newKeys << openSshKey;
		}

		FileSystemUtils::writeLinesToFile( knownHostsFile, newKeys );
}

QString HostFileUtils::getIpAddress( const QString& host )
{
    QHostInfo hostInfo = QHostInfo::fromName( host ); // blocking lookup

    if (hostInfo.error() != QHostInfo::NoError)
    {
        qWarning() << "IP address lookup failed:" << hostInfo.errorString();
        return QString();
    }
    
    return hostInfo.addresses().first().toString(); // use the first IP address
}
