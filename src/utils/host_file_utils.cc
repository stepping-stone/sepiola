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
    QString puttyKey;

#ifdef Q_OS_WIN32
    // Read the SshHostKey fingerprint form the registry.
    QSettings puttySshHostKey("HKEY_CURRENT_USER\\Software\\SimonTatham\\PUTTY\\SshHostKeys", QSettings::NativeFormat);
    QVariant puttyKeyValue = puttySshHostKey.value(QString("rsa2@22:%2".arg(host)));

    if (puttyKeyValue.isNull()) {
        qWarning() << "Can not find the given putty key in the Windows registry";
        return;
    }

    puttyKey = puttyKeyValue.toString();
#else
    puttyKey = getPuttyKey(host, sshhostkeysFileName);

    if( puttyKey.isEmpty() )
    {
        qDebug() << "no putty key found for host " << host;
        return;
    }
    // on *nix, they key is of the format "rsa2@<port>:<host> 0x10001,0xcd7843370db6046..."
    puttyKey = puttyKey.split(' ').at(1); // split it and continue with the actual host-key part
#endif

    QString sshKey = convertPuttyKey(puttyKey, host);
    addOpenSshKey(sshKey, host, sshKnownHostsFileName);
}

QString HostFileUtils::getPuttyKey( const QString& host, const QString& sshhostkeysFileName )
{
    QStringList allKeys = FileSystemUtils::readLinesFromFile( sshhostkeysFileName );

    QString hostSearchPattern = ":" + host + " "; //rsa2@22:host 0x23,0x

    foreach (QString key, allKeys)
        if (key.contains( hostSearchPattern))
            return key;

    qDebug() << "key not found";
    return QString();
}

QString HostFileUtils::convertPuttyKey( const QString& puttyData, const QString& host )
{
    // the format of the puttyKeyString is:
    //     0x10001,0xcd7843370db6046...

    // build the list of subkeys first
    // the first element for openSSH is again the keytype
    QList<QByteArray> subkeys({"ssh-rsa"});
    for (const QString& entry: puttyData.split(','))
        subkeys.append(QByteArray::fromHex(entry.mid(2).toAscii()));

    // the actual key has to be left padded with a 0, but why?
    Q_ASSERT(subkeys.size() == 3); // make sure we really have only 3 elements, otherwise our assumptions are incorrect
    subkeys[2].insert(0, '\0');

    // merge the subkeys, the format expected by OpenSSH is:
    // <n1><c1_1><c1_2>...<c1_n1><n2><c2_1><c2_2>...<c2_n2>...
    // where the n's are uint32 in big endian order and the c's are single bytes
    // therefore the "magic" 00 00 00 07 ... at the beginning of each ssh-rsa key type: len("ssh-rsa") = 7
    QByteArray key;
    for (const auto& entry: subkeys)
    {
        quint32 length = qToBigEndian(entry.size()); // make sure the length has the correct byte order
        key.append(static_cast<const char*>(static_cast<void*>(&length)), 4); // a uint32 is guaranteed to be 4 bytes
        key.append(entry);
    }

    // the SSH format is: <hostname>,<ip> <key format> <key data (base64-encoded)>:
    return QString("%1,%2 %3 %4")
        .arg(host)
        .arg(getIpAddress(host))
        .arg("ssh-rsa")
        .arg(key.toBase64().data());
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
