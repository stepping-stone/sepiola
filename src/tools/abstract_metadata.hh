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

#ifndef ABSTRACT_ACL_HH
#define ABSTRACT_ACL_HH

#include <QString>
#include <QFileInfo>

#include "tools/abstract_progress.hh"
#include "tools/abstract_rsync.hh"

/**
 * The AbstractMetadata class provides methods for getting and setting file and directory meta data
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: bsantschi $ $Date: 2008/04/25 05:42:16 $ $Revision: 1.9 $
 */
class AbstractMetadata : public AbstractProgress
{
public:

	/**
	 * Destroys the AbstractMetadata
	 */
	virtual ~AbstractMetadata();
	
	/**
	 * Creates a metadata file for the given file and directory list
	 * @param processedItems list of file and directory names
	 * @return the full name of the created metadata file
	 */
	virtual QString getMetadata( const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems ) = 0;
	
	/**
	 * Merges the data from the new and the existing metadata file and deletes items if necessary
	 * @param newMetadataFileName the name of the new file
	 * @param currentMetadataFileName the name of the current file
	 * @param processedItems list of file and directory names
	 */
	virtual void mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems ) = 0;
	
	/**
	 * Sets the metadata from a metadata file for the items in the list
	 * @param metadataFile file containing the metadata
	 * @param downloadedItems list of file and directory names for setting the ACL's
	 * @param downloadDestination destination path of the restore
	 */
	virtual void setMetadata( const QFileInfo& metadataFile, const QStringList& downloadedItems, const QString& downloadDestination ) = 0;
	
	/**
	 * Returns a list of files and directories from a given ACL file
	 * @param metadataFile file containing the metadata
	 */
	 virtual QStringList extractItems( const QFileInfo& metadataFile ) = 0;
};

inline AbstractMetadata::~AbstractMetadata()
{
}

#endif
