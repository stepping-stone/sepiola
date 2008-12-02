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

#ifndef HOST_FILE_UTILS_HH
#define HOST_FILE_UTILS_HH

#include <QString>

/**
 * The HostFileUtils class provides convenience methods for reading
 * and writing the host files for putty and open ssh
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class HostFileUtils
{
private:
	HostFileUtils();
	virtual ~HostFileUtils();

public:
	
	/**
	 * Converts the putty host key to the open ssh key format and adds
	 * the key to the known_hosts file
	 */
	static void addPuttyKeyToOpenSshKeyFile();
	
	/**
	 * Tests the getPuttyKey method
	 */
	static void testGetPuttyKey();

	/**
	 * Tests the addHostToHostFile method
	 */
	static void testConvertPuttyKey();

	/**
	 * Tests the addPuttyKeyToOpenSshKeyFile method
	 */
	static void testAddPuttyKeyToOpenSshKeyFile();

private:	
	/**
	 * Gets the putty key for a given host
	 */
	static QString getPuttyKey( const QString& host );

	/**
	 * Converts the given putty key to the open ssh key
	 */
	static QString convertPuttyKey(const QString& puttyKey, const QString& host);
	/**
	 * Adds the given key to the host file
	 */
	static void addOpenSshKey( const QString& openSshKey, const QString& host );
	
	/**
	 * Gets the IP address for the given host name
	 */
	static QString getIpAddress( const QString& host );
};
#endif
