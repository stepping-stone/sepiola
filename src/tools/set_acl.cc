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
#include <QMap>
#include <QDir>

#include "tools/set_acl.hh"
#include "utils/file_system_utils.hh"

SetAcl::SetAcl(const QString& setAcl) :
    setAclName(setAcl)
{
}

SetAcl::~SetAcl()
{
}

const QString SetAcl::ITEM_NAME_PREFIX = "\"\\\\?\\";
const uint SetAcl::ITEM_NAME_PREFIX_SIZE = 5;

QString SetAcl::getMetadata(const QString& aclFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems, QString* /*warnings*/ )
{
	qDebug() << "SetAcl::getMetadata( processedItems )";
	QStringList arguments;
	arguments << "-ot" << "file" << "-rec" << "no" << "-actn" << "list" << "-lst" << "f:sddl;w:d,s,o,g" << "-bckp" << aclFileName;
	createProcess( setAclName, arguments );
	start();
	setTextModeEnabled(true);
	//waitForStarted();
/*	QFile setAclInFile(settings->getApplicationDataDir() + "setacl-input.txt");
	setAclInFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream setAclInStream(&setAclInFile);
	setAclInStream.setCodec("UTF-16");
	setAclInStream.setGenerateByteOrderMark(true); */
	QTextStream outStream;
	retrieveStream(&outStream);
	outStream.setCodec("UTF-16");
	for( int i=0; i<processedItems.size(); i++ )
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>  processedItem = processedItems.at( i );
		if( processedItem.second != AbstractRsync::DELETED )
		{
			//emit infoSignal( QObject::tr( "Getting metadata for %1" ).arg( processedItem.first ) );
			outStream << processedItem.first << endl;
			//setAclInStream << processedItem.first << endl;
			waitForBytesWritten();
		}
	}

	closeWriteChannel();
	QString errors;
	if (!waitForFinished())
	{
		qDebug() << "Process state: " << state();
		//errors = "Error or timeout waiting for SetACL";
	}
	else
	{
		errors = readAllStandardError();
	}
	if ( errors != "" )
	{
		qWarning() << "Error occurred while getting ACL's: " << errors;
		throw ProcessException( QObject::tr( "Error occurred while getting ACL's:\n" ) + errors );
	}
	return aclFileName;
}

void SetAcl::mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
{
	qDebug() << "SetAcl::mergeMetadata( " << newMetadataFileName.absoluteFilePath() << ", " << currentMetadataFileName.absoluteFilePath() << ")";

	QMap<QString, QString> aclMap; // key: file or dir name, value: acl data (one line)

	// read all acl's from the current acl file and put them in a map
	populateMapFromFile( currentMetadataFileName, &aclMap );

	// delete all acl's for deleted items
	for( int i=0; i<processedItems.size(); i++ )
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>  processedItem = processedItems.at( i );
		if( processedItem.second == AbstractRsync::DELETED )
		{
			aclMap.remove( processedItem.first );
		}
	}

	// read all acl's from the new acl file and put them in the map. if there is already an entry, replace it
	populateMapFromFile( newMetadataFileName, &aclMap );

	writeMapContentToFile( aclMap, currentMetadataFileName );
}

void SetAcl::setMetadata( const QFileInfo& metadataFileName, const QStringList& downloadedItems, const QString& downloadDestination )
{
	qDebug() << "SetAcl::setMetadata( " << metadataFileName.absoluteFilePath() << ", ..., " << downloadDestination << " )";

	emit infoSignal( QObject::tr( "Preparing metadata" ) );

	QMap<QString, QString> aclMap; // key: file or dir name, value: acl data (one line)
	QMap<QString, QString> newAclMap;

	populateMapFromFile( metadataFileName, &aclMap );

	foreach (QString item, downloadedItems)
	{
		QString aclValue = aclMap.take(item);
		if (aclValue.isEmpty()) continue;

		//QString itemName = extractItemNameFromAclValue( aclValue );
		if ( downloadDestination == "/" )
		{
			newAclMap.insert(item, aclValue);
		}
		else
		{
			FileSystemUtils::convertToServerPath( &item );
			QString absoluteItemPath = QDir::toNativeSeparators( downloadDestination + item);
			aclValue = ITEM_NAME_PREFIX + absoluteItemPath + aclValue.right( aclValue.size() - item.size() - ITEM_NAME_PREFIX_SIZE );
			newAclMap.insert( absoluteItemPath, aclValue ); // insert new
			/*if (FileSystemUtils::isDir(absoluteItemPath))
			{
				emit infoSignal( QObject::tr( "Setting metadata for %1" ).arg( absoluteItemPath ) );
			}*/
		}
	}

	aclMap.clear();

	writeMapContentToFile( newAclMap, metadataFileName );
	newAclMap.clear();

	// apply acl
	QStringList arguments;
	arguments << "-ot" << "file" << "-rec" << "no" << "-actn" << "restore" << "-bckp" << metadataFileName.absoluteFilePath();

	createProcess( setAclName , arguments );

	setReadChannel(QProcess::StandardOutput);
	/*setProcessChannelMode(QProcess::MergedChannels);
	setTextModeEnabled(true);
	QTextStream inputStream;
	retrieveStream(&inputStream);*/
	//inputStream.setCodec("UTF-16");

	start();

	QByteArray line;
	while( blockingReadLine( &line, 2147483647 ) )
	{
		emit infoSignal( line.trimmed() );
		//qDebug() << line;
	}

	/*QString line;
	for (int i = 0; i < 10; i++ )
	{
		qDebug() << inputStream.readLine();
	}*/
	/*while (!inputStream.atEnd())
	{
		line = inputStream.readLine();
		qDebug() << line;
	}*/

	QString errors;
	if (!waitForFinished())
	{
		qDebug() << "Process state: " << state();
		// errors = "Error or timeout waiting for SetACL";
	}
	else
	{
		errors = readAllStandardError();
	}
	if ( errors != "" )
	{
		qWarning() << "Error occurred while setting ACL's: " << errors;
		throw ProcessException( QObject::tr( "Error occurred while setting ACL's:\n" ) + errors );
	}
}

QStringList SetAcl::extractItems( const QFileInfo& metadataFile )
{
	qDebug() << "SetAcl::extractItems( " << metadataFile.absoluteFilePath() << " )";

	QMap<QString, QString> aclMap; // key: file or dir name, value: acl data (one line)
	populateMapFromFile( metadataFile, &aclMap );
	return aclMap.keys();
}

void SetAcl::populateMapFromFile( const QFileInfo& aclFileName, QMap<QString, QString>* aclMap)
{
	qDebug() << "SetAcl::populateMapFromFile( " << aclFileName.absoluteFilePath() << ", aclMap )";

	QFile aclFile( aclFileName.absoluteFilePath() );
	if ( !aclFile.open( QIODevice::ReadOnly ) )
	{
		qWarning() << "Can not read from file " << aclFileName.absoluteFilePath();
		return;
	}
	else
	{
		qDebug() << "Parsing file " << aclFileName.absoluteFilePath();
	}

	QTextStream textStream( &aclFile );
	textStream.setCodec( "UTF-16" );
	textStream.setGenerateByteOrderMark(true);
	while ( !textStream.atEnd() )
	{
		QString line = textStream.readLine();
		if ( !line.startsWith( ITEM_NAME_PREFIX ) )
		{
			qCritical() << "Unsupported ACL file format";
			return;
		}
		QString itemName = extractItemNameFromAclValue( line );
		aclMap->insert( itemName, line ); // overrides the element if it already exists
	}
	aclFile.close();
}

QString SetAcl::extractItemNameFromAclValue( const QString aclValue )
{
	int itemNameStartPosition = ITEM_NAME_PREFIX.size();
	int itemNameEndPosition = aclValue.indexOf( "\"", itemNameStartPosition ) - itemNameStartPosition;
	return aclValue.mid( itemNameStartPosition, itemNameEndPosition );
}

void SetAcl::writeMapContentToFile( const QMap<QString, QString>& aclMap, const QFileInfo& aclFileName)
{
	qDebug() << "SetAcl::writeMapContentToFile( aclMap, " << aclFileName.absoluteFilePath() << " )";

	// if the file already exists, delete and recreate the acl file, then write the content of the map to the file
	QFile aclFile( aclFileName.absoluteFilePath() );
	if ( aclFile.exists() )
	{
		aclFile.remove();
	}
	if ( !aclFile.open( QIODevice::WriteOnly ) )
	{
		qWarning() << "Can not write to file " << aclFileName.absoluteFilePath();
		return;
	}
	aclFile.setTextModeEnabled(true);
	QTextStream textStream( &aclFile );
	textStream.setCodec( "UTF-16" );
	textStream.setGenerateByteOrderMark(true);
	QList<QString> aclValues = aclMap.values();
	foreach ( QString aclValue, aclValues )
	{
		textStream << aclValue << endl;
	}
	textStream.flush();
	aclFile.close();
}
