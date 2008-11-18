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

#include <QDebug>
#include <QFile>
#include <QFlags>
#include <QTextStream>

#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "tools/unix_permissions.hh"
#include "utils/extended_file.hh"
#include "utils/unicode_text_stream.hh"

UnixPermissions::UnixPermissions()
{
}

UnixPermissions::~UnixPermissions()
{
}

QString UnixPermissions::getMetadata( const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
{
	qDebug() << "UnixPermissions::getMetadata( processedItems )";
	Settings* settings = Settings::getInstance();

	// metadata format: filename \n owner \n group \n permission
	QString metadataFileName = settings->getApplicationDataDir() + settings->getTempMetadataFileName();
	QFile metadataFile( metadataFileName );
	if ( !metadataFile.open( QIODevice::WriteOnly ) )
	{
		qWarning() << "Can not write to file: " << metadataFileName;
		return metadataFileName;
	}

	UnicodeTextStream out( &metadataFile );

	for( int i=0; i<processedItems.size(); i++ )
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>  processedItem = processedItems.at( i );
		if( processedItem.second != AbstractRsync::DELETED )
		{
			qDebug() << "Getting metadata for " << processedItem.first;
			emit infoSignal( QObject::tr( "Getting metadata for %1" ).arg( processedItem.first ) );
			ExtendedFile fileInfo( processedItem.first );
			QString fileName = fileInfo.fileName();
			QString owner = fileInfo.owner();
			QString group = fileInfo.group();
			int permissions = fileInfo.permissions();
			out << fileName << "\n" << owner << "\n" << group << "\n" << permissions << "\n\n";
		}
	}
	metadataFile.close();
	return metadataFileName;
}

void UnixPermissions::mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
{
	qDebug() << "UnixPermissions::mergeMetadata( " << newMetadataFileName.absoluteFilePath() << ", " << currentMetadataFileName.absoluteFilePath() << ")";

	QMap<QString, QStringList> metadataMap; // key: file or dir name, value: acl data (four lines)

	// read all metadata from the current metadata file and put them in a map
	populateMapFromFile( currentMetadataFileName, &metadataMap );

	// delete all metadata for deleted items
	for( int i=0; i<processedItems.size(); i++ )
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>  processedItem = processedItems.at( i );
		if( processedItem.second == AbstractRsync::DELETED )
		{
			metadataMap.remove( processedItem.first );
		}
	}

	// read all metadata from the new metadata file and put them in the map. if there is already an entry, replace it
	populateMapFromFile( newMetadataFileName, &metadataMap );

	writeMapContentToFile( metadataMap, currentMetadataFileName );
}

void UnixPermissions::setMetadata( const QFileInfo& metadataFileName, const QStringList& downloadedItems, const QString& downloadDestination )
{
	qDebug() << "UnixPermissions::setMetadata( " << metadataFileName.absoluteFilePath() << ", " << downloadedItems << " )";

	QMap<QString, QStringList>* metadataMap = new QMap<QString, QStringList>(); // key: file or dir name, value meta data (four lines)
	populateMapFromFile( metadataFileName, metadataMap );

	QString errors;
	foreach ( QString itemName, metadataMap->keys() )
	{
		if ( downloadedItems.contains( itemName.right( itemName.size() -1 ) ) ) // itemName without prepended slash
		{
			QString fileName;
			if ( downloadDestination != "/" )
			{
				fileName = downloadDestination + itemName;
			}
			else
			{
				fileName = itemName;
			}
			// set metadata
			qDebug() << "setting meta data for " << fileName;
			emit infoSignal( QObject::tr( "Setting metadata for %1" ).arg( fileName ) );
			ExtendedFile file( fileName );
			QStringList metadata = metadataMap->value( itemName );
			QString owner = metadata.at( 1 );
			QString group = metadata.at( 2 );
			int permissions = QVariant( metadata.at( 3 ) ).toInt();
			QFlags<QFile::Permission> permission( permissions );
			file.setPermissions( permission );
			if( !file.setOwnerAndGroup( owner, group ) )
			{
				errors.append( QObject::tr( "Cannot change owner/group for %1\n" ).arg( fileName ) );
			}
		}
	}
	if ( errors != "" )
	{
		qWarning() << "Error occurred while setting permissionss: " << errors;
		throw ProcessException( QObject::tr( "Error occurred while setting permissions:\n" ) + errors );
	}

}

QStringList UnixPermissions::extractItems( const QFileInfo& metadataFile )
{
	qDebug() << "UnixPermissions::extractItems( " << metadataFile.absoluteFilePath() << " )";

	QMap<QString, QStringList>* metadataMap = new QMap<QString, QStringList>(); // key: file or dir name, value: acl data (four lines)
	populateMapFromFile( metadataFile, metadataMap );
	return metadataMap->keys();
}

void UnixPermissions::populateMapFromFile( const QFileInfo& metadataFileName, QMap<QString, QStringList>* metadataMap )
{
	qDebug() << "UnixPermissions::populateMapFromFile( " << metadataFileName.absoluteFilePath() << ", metadataMap )";

	QFile metadataFile( metadataFileName.absoluteFilePath() );
	if ( !metadataFile.open( QIODevice::ReadOnly ) )
	{
		qWarning() << "Can not read file " << metadataFileName.absoluteFilePath();
		return;
	}

	UnicodeTextStream in( &metadataFile );
	while ( !in.atEnd() )
	{
		QString fileName = in.readLine();
		QStringList metadata;
		metadata << fileName;
		metadata << in.readLine(); // owner
		metadata << in.readLine(); // group
		metadata <<	in.readLine(); // permissions
		metadataMap->insert( fileName, metadata );
		in.readLine(); // empty line
	}
	metadataFile.close();
}

void UnixPermissions::writeMapContentToFile( const QMap<QString, QStringList>& metadataMap, const QFileInfo& metadataFileName )
{
	qDebug() << "UnixPermissions::writeMapContentToFile( metadataMap, " << metadataFileName.absoluteFilePath() << " )";

	// if the file already exists, delete and recreate the acl file, then write the content of the map to the file
	QFile metadataFile( metadataFileName.absoluteFilePath() );
	if ( metadataFile.exists() )
	{
		metadataFile.remove();
	}
	if ( !metadataFile.open( QIODevice::WriteOnly ) )
	{
		qWarning() << "Can not write to file " << metadataFileName.absoluteFilePath();
		return;
	}
	UnicodeTextStream out( &metadataFile );
	QList<QString> metadataKeys = metadataMap.keys();
	foreach ( QString metadataKey, metadataKeys )
	{
		QStringList metadataValues = metadataMap.value( metadataKey );
		foreach( QString metadataValue, metadataValues )
		{
			out << metadataValue << "\n";
		}
		out << "\n";
	}
	metadataFile.close();
}

void UnixPermissions::testGetMetadata()
{
	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems;
	processedItems << qMakePair( QString( "/Users/test/tools/putty-0.60.tar.gz" ), AbstractRsync::UPLOADED );
	processedItems << qMakePair( QString( "/Users/test/tools/info.txt" ), AbstractRsync::DELETED );
	UnixPermissions unixPermissions;
	unixPermissions.getMetadata( processedItems );
}

void UnixPermissions::testInsertMetadata()
{
	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems;
	processedItems << qMakePair( QString( "/Users/test/tools/putty-0.60.tar.gz" ), AbstractRsync::UPLOADED );
	processedItems << qMakePair( QString( "/Users/test/tools/info.txt" ), AbstractRsync::DELETED );
	UnixPermissions unixPermissions;
	QString metadataFile = unixPermissions.getMetadata( processedItems );
	unixPermissions.mergeMetadata( metadataFile, metadataFile, processedItems );
}

void UnixPermissions::testSetMetadata()
{
	Settings* settings = Settings::getInstance();
	QString aclFile = settings->getApplicationDataDir() + settings->getMetadataFileName();

	QStringList downloadedFiles;
	downloadedFiles << "Users/test/tools/putty-0.60.tar.gz";

	UnixPermissions unixPermissions;
	unixPermissions.setMetadata( aclFile, downloadedFiles, "/tmp" );
}

namespace
{
	int dummy = TestManager::registerTest( "unixPermissions_testGetMetadata", UnixPermissions::testGetMetadata );
	int dummy2 = TestManager::registerTest( "unixPermissions_testSetMetadata", UnixPermissions::testSetMetadata );
	int dummy3 = TestManager::registerTest( "unixPermissions_testInsertMetadata", UnixPermissions::testInsertMetadata );
}
