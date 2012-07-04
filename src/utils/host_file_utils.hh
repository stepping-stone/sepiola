/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008, 2012  stepping stone GmbH
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

#ifndef HOST_FILE_UTILS_HH
#define HOST_FILE_UTILS_HH

#include <QString>

/**
 * The HostFileUtils class provides convenience methods for reading
 * and writing the host files for Putty and OpenSSH
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class HostFileUtils
{
public:

    /**
     * Copy a Putty host key to the OpenSSH known_hosts file.
     * @param[in] host  Hostname for which to copy the key
     * @param[in] sshhostkeysFileName  Path to Puttys sshhostkeys file
     * @param[in] sshKnownHostsFileName  Path to OpenSSHs known_hosts file
     *
     * Uses the other methods in this class to lookup the key for a given host
     * in Puttys sshhostkeys, converts it to the OpenSSH format and writes it
     * to OpenSSHs known_hosts file.
     */
	static void addPuttyKeyToOpenSshKeyFile( const QString& host, const QString& sshhostkeysFileName, const QString& sshKnownHostsFileName );
	
    /**
     * Gets the putty key for a given host.
     * @param[in] host  The hostname for which to retrieve the key
     * @param[in] ssHhostKeysFileName  Path to the Putty sshhostkeys file
     *
     * @return The key if found, otherwise a null string
     *
     */
	static QString getPuttyKey( const QString& host, const QString& ssHhostKeysFileName );

    /**
     * Convert the given Putty key to the OpenSSH key format.
     * @param[in] puttyKey  The key in Putty format
     * @param[in] host  Hostname matching the key
     *
     * @return OpenSSH known_hosts key format string
     *
     */
	static QString convertPuttyKey( const QString& puttyKey, const QString& host );

    /**
     * Add the given key to the host file.
     * @param[in] openSshKey  The key to add
     * @param[in] host  Hostname matching the given key
     * @param[in] knownHostsFile  Path to the known_hosts file
     *
     */
	static void addOpenSshKey( const QString& openSshKey, const QString& host, const QString& knownHostsFile );
	
    /**
     * Lookup the IP address for the given host name.
     * @param[in] host  The host to look the IP up for 
     *
     * @return The first IP found for the hostname or a null string if something went wrong.
     *
     * This is basically a wrapper around QHostInfo::fromName.
     */
	static QString getIpAddress( const QString& host );
};
#endif
