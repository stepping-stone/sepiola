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

#ifndef POSIX_ACL_HH
#define POSIX_ACL_HH

#include <QString>

#include "tools/abstract_metadata.hh"
#include "tools/abstract_rsync.hh"
#include "process.hh"

/**
 * The PosixAcl class provides methods for using getfacl and setfacl tools
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class PosixAcl : public AbstractMetadata, public Process
{
public:

	/**
	 * Constructs a PosixAcl
	 */
    PosixAcl(const QString& getfacl, const QString& setfacl);
	/**
	 * Destroys the PosixAcl
	 */
	virtual ~PosixAcl();

	/**
	 * @see AbstractMetadata::getMetadata( const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
	 */
    QString getMetadata(
        const QString& aclFileName,
        const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems,
        QString* warnings);

	/**
	 * @see AbstractMetadata::mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
	 */
	void mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems );

	/**
	 * @see AbstractMetadata::setMetadata( const QFileInfo& metadataFile, const QStringList& downloadedItems, const QString& downloadDestination )
	 */
	void setMetadata( const QFileInfo& metadataFile, const QStringList& downloadedItems, const QString& downloadDestination );

	/**
	 * @see AbstractMetadata::extractItems( const QFileInfo& metadataFile )
	 */
	QStringList extractItems( const QFileInfo& metadataFile );

protected:
    friend class TestPosixAcl;
	static QString unescapeOctalCharacters( const QString& escapedString, bool* ok );

private:
	static const QString ITEM_NAME_PREFIX;

	void populateMapFromFile( const QFileInfo& aclFileName, QMap<QString, QStringList>* aclMap);
	void writeMapContentToFile( const QMap<QString, QStringList>& aclMap, const QFileInfo& aclFileName);

    const QString getfaclName;
    const QString setfaclName;
};

#endif
