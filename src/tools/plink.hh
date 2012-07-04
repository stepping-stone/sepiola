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

#ifndef PLINK_HH
#define PLINK_HH

#include "tools/abstract_ssh.hh"
#include "tools/process.hh"

/**
 * The Plink class provides methods for using the plink tool
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Plink : public AbstractSsh, public Process
{

public:	

	/**
	 * Constructs a Plink
	 */
	Plink();
	
	/**
	 * Destroys the Plink
	 */
	virtual ~Plink();

	/**
	 * @see AbstractSsh::uploadToMetaFolder( const QFileInfo& file, bool append )
	 */
	void uploadToMetaFolder( const QFileInfo& file, bool append );
	
	/**
	 * @see AbstractSsh::generateKeys( const QString& password )
	 */
	bool generateKeys( const QString& password );
	
	/**
	 * @see AbstractSsh::loginWithKey()
	 */
	bool loginWithKey();
	
	/**
	 * @see AbstractSsh::assertCorrectFingerprint()
	 */
	bool assertCorrectFingerprint();

    bool assertCorrectFingerprint(const QString& userName, const QString& serverName, const QString& savedKey);
	
	/**
	 * @see AbstractSsh::getServerQuotaValues()
	 */
	QList<int> getServerQuotaValues();
	
	/**
	 * Tests the generateKeys method
	 * @see generateKeys()
	 */
	static void testGenerateKeys();
	
	/**
	 * Tests the loginWithKey method
	 */
	static void testLoginWithKey();
	
	/**
	 * Tests the assertCorrectFingerprint method
	 */
	static void testAssertCorrectFingerprint();

private:
	static const QString PUTTY_HEADER_FIRST_LINE;
	static const QString OPEN_SSH_HEADER_FIRST_LINE;
	static const QString LOGIN_ECHO_MESSAGE;
	static const QString START_PRIVATE_KEY_ECHO_MESSAGE;
	static const QString END_PRIVATE_KEY_ECHO_MESSAGE;
	static const QString START_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE;
	static const QString END_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE;

    const QString plinkName;

	static QString extractKey( const QStringList& keyLines, const QString& startLine, const QString& endLine );
};

#endif
