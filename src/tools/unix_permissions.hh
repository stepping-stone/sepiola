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

#ifndef UNIX_PERMISSIONS_HH
#define UNIX_PERMISSIONS_HH

#include <QString>
#include <QStringList>

#include "tools/abstract_metadata.hh"
#include "tools/process.hh"
#include "tools/filesystem_snapshot.hh"

/**
 * The UnixPermissions class provides methods for getting and setting Unix permissions.
 * This class is currently used for Mac OS X.
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class UnixPermissions : public AbstractMetadata, public Process
{
public:
	/**
	 * @see AbstractMetadata::getMetadata( const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
	 */
	virtual QString getMetadata(
		const QString& metadataFileName,
		const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems,
		const FilesystemSnapshot*,
		QString* warnings = nullptr);

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

	/**
	 * Tests the getMetadata method
	 */
	static void testGetMetadata();

	/**
	 * Tests the insertMetadata method
	 */
	static void testInsertMetadata();

	/**
	 * Tests the setMetadata method
	 */
	static void testSetMetadata();

private:
	void populateMapFromFile( const QFileInfo& metadataFileName, QMap<QString, QStringList>* metadataMap);
	void writeMapContentToFile( const QMap<QString, QStringList>& metadataMap, const QFileInfo& metadataFileName );
};

#endif
