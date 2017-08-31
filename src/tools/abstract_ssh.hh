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

#ifndef ABSTRACT_SSH_HH
#define ABSTRACT_SSH_HH

#include <QFileInfo>

#include "model/restore_name.hh"
#include "tools/abstract_informing_process.hh"

/**
 * The AbstractSsh class provides methods for using SSH
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class AbstractSsh : public AbstractInformingProcess
{
public:

	/**
	 * Destroys the AbstractSsh
	 */	
	virtual ~AbstractSsh();
	
	/**
	 * Uploads a given file.
	 * @param file file to upload
	 * @param append indicates whether the file will be appended or overridden 
	 */
	virtual void uploadToMetaFolder( const QFileInfo& file, bool append ) = 0;
	
	/**
	 * Generates a public and a private SSH key on the server as follows:
	 * - The public key will be directly appended to the authorized_keys file.
	 * - The private keys (putty and openssh) will be downloaded and saved
	 * - All keys will be deleted on the server 
	 * @param password the user's password
	 * @return flag if key generation was successful
	 */
	virtual bool generateKeys( const QString& password ) = 0;
	
	/**
	 * Tests if the user can login to the server with a key
	 */
	virtual bool loginWithKey() = 0;
	
	/**
	 * Asserts that the server has the same fingerprint as set in the settings
	 */
	virtual bool assertCorrectFingerprint() = 0;
	
	/**
	 * calls a script on the server which returns quota-values (quota, 
	 * backupSize, snapshotSize) and returns the values in a QList<int>
	 */
	virtual QList<int> getServerQuotaValues() = 0;
};

inline AbstractSsh::~AbstractSsh()
{
}

#endif
