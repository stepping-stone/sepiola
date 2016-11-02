/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2012 stepping stone GmbH
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

#include "utils/host_file_utils.hh"
#include "utils/file_system_utils.hh"

#include <QSettings>
#include <QDebug>
#include <QtEndian>
#include <QHostInfo>
#include <QByteArray>
#include <QStringList>

void HostFileUtils::addPuttyKeyToOpenSshKeyFile( const QString& host, const QString& sshhostkeysFileName, const QString& sshKnownHostsFileName )
{
#ifdef Q_OS_WIN32
    // Read the SshHostKey fingerprint form the registry.
    QString rsaPrefix("0x23,0x");
    QSettings puttySshHostKey("HKEY_CURRENT_USER\\Software\\SimonTatham\\PUTTY\\SshHostKeys", QSettings::NativeFormat);
    QString hostKeyValue = puttySshHostKey.value("rsa2@22:kvm-0003.stepping-stone.ch", "keyNotFound").toString();
    if (hostKeyValue.compare("keyNotFound") == 0) {
      qWarning() << "Can not find the given putty key in the Windows registry";
      return;
    }
    hostKeyValue.remove(0 , rsaPrefix.length());
    QString sshKey = convertPuttyKey(hostKeyValue, host);
    addOpenSshKey( sshKey, host, sshKnownHostsFileName);
#else
	QString puttyKey = getPuttyKey( host, sshhostkeysFileName );
	if( puttyKey.isEmpty() )
	{
		qDebug() << "no putty key found for host " << host;
	}
	else
	{
        int prefix = 16 + host.length(); //rsa2@22:host 0x23,0x
        puttyKey.remove(0,  prefix);
		QString sshKey = convertPuttyKey( puttyKey, host );
		addOpenSshKey( sshKey, host, sshKnownHostsFileName );
	}
#endif
}

QString HostFileUtils::getPuttyKey( const QString& host, const QString& sshhostkeysFileName )
{
	QStringList allKeys = FileSystemUtils::readLinesFromFile( sshhostkeysFileName );

    QString hostSearchPattern = ":" + host + " "; //rsa2@22:host 0x23,0x
    foreach( QString key, allKeys )
    {
        if( key.contains( hostSearchPattern )) {
            return key;
        }
    }
    qDebug() << "key not found";
    return QString();
}

QString HostFileUtils::convertPuttyKey( const QString& puttyKeyString, const QString& host )
{
    QByteArray puttyKey;
    puttyKey.append(puttyKeyString);

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

void HostFileUtils::addOpenSshKey( const QString& openSshKey, const QString& host, const QString& knownHostsFile )
{
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
