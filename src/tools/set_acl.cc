/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
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

#include "settings/settings.hh"
#include "tools/set_acl.hh"
#include "utils/file_system_utils.hh"

typedef QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> UploadedFile;

SetAcl::SetAcl(const QString& setAcl) :
    setAclName(setAcl)
{
}

SetAcl::~SetAcl()
{
}

const QString SetAcl::ITEM_NAME_PREFIX = "\"\\\\?\\";
const uint SetAcl::ITEM_NAME_PREFIX_SIZE = 5;

QString SetAcl::getMetadata(const QString& aclFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems, const FilesystemSnapshot* fsSnapshot, QString* )
{
    Settings* settings = Settings::getInstance();
	qDebug() << "SetAcl::getMetadata( processedItems )";
	QStringList arguments;
    arguments << "-ot" << "file" << "-rec" << "no" << "-actn" << "list" << "-lst" << "f:sddl;w:d,s,o,g" << "-bckp" << aclFileName;
	createProcess( setAclName, arguments );
	start();
	setTextModeEnabled(true);
	QTextStream outStream;
	retrieveStream(&outStream);
	outStream.setCodec("UTF-16");

    //Convert the processedItems to windows paths: /C/User/USERNAME  -> C:\User\USERNAME
    QList< UploadedFile > localPathsWindows;
    QList< UploadedFile > absolutUncPaths;
    foreach( UploadedFile  item, processedItems ) {
        FileSystemUtils::convertToLocalPath(&item.first);
        localPathsWindows.append(item);
        absolutUncPaths.append(item);
    }
    if (settings->getDoSnapshot()) {
        // Convert the item paths to unc. read the acl form the shadow copy if a shadwo copy was taken.
        QString absolutePath;
        absolutUncPaths.clear();
        foreach(FilesystemSnapshotPathMapper mapper, fsSnapshot->getSnapshotPathMappers()) {
            foreach( UploadedFile  item, localPathsWindows) {
                if (item.first.startsWith(mapper.getPartition())) {
                    absolutePath = mapper.toAbsUncPath(item.first);
                    absolutePath.remove("\\\\?\\");
                    UploadedFile file( absolutePath, item.second);
                    absolutUncPaths.append(file);
                }
            }
        }
    }
    foreach (UploadedFile item, absolutUncPaths) {
        if(item.second != AbstractRsync::DELETED) {
            outStream << item.first << endl;
            waitForBytesWritten();
        }
    }
	closeWriteChannel();
	QString errors;
	if (!waitForFinished())
	{
        qDebug() << "Process state: " << state();
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

    if(settings->getDoSnapshot())
      replaceUncPathsByLocalPathsInMetadataFile(aclFileName, fsSnapshot);

	return aclFileName;
}

void SetAcl::mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
{
    //Convert the processedItem-path to the correspondign operating system: E.g. windows: /C/User/USERNAME  -> C:\User\USERNAME
    QList<UploadedFile> localPathsWindows;
    foreach( UploadedFile item, processedItems ) {
        FileSystemUtils::convertToLocalPath(&item.first);
        localPathsWindows.append(item);
    }
	qDebug() << "SetAcl::mergeMetadata( " << newMetadataFileName.absoluteFilePath() << ", " << currentMetadataFileName.absoluteFilePath() << ")";

	QMap<QString, QString> aclMap; // key: file or dir name, value: acl data (one line)

	// read all acl's from the current acl file and put them in a map
	populateMapFromFile( currentMetadataFileName, &aclMap );

    // delete all acl's for deleted items
    foreach (UploadedFile item, localPathsWindows) {
        if (item.second == AbstractRsync::DELETED) {
            aclMap.remove( item.first );
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
	start();

	QByteArray line;
	while( blockingReadLine( &line, 2147483647 ) )
	{
		emit infoSignal( line.trimmed() );
	}
	QString errors;
	if (!waitForFinished())
	{
        qDebug() << "Process state: " << state();
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

void SetAcl::replaceUncPathsByLocalPathsInMetadataFile(const QFileInfo &Filename,  const FilesystemSnapshot *snapshot) const {
    QStringList aclMetadataFromUncPath;
    QStringList aclMetadataFromLocalPath;
    QFile origMetadataFile(Filename.absoluteFilePath());
    QTextStream readStream(&origMetadataFile);
    readStream.setCodec("UTF-16");

    if(!origMetadataFile.open(QIODevice::ReadWrite)) {
        qWarning() << "Can not write to file " << origMetadataFile.fileName();
        return;
    } else {
        while (!readStream.atEnd()) {
            QString line = readStream.readLine();
            aclMetadataFromUncPath.append(line);
        }
        origMetadataFile.close();
    }
    foreach(FilesystemSnapshotPathMapper mapper, snapshot->getSnapshotPathMappers()) {
        foreach(QString line, aclMetadataFromUncPath) {
            line.remove(0, 1); // remove the leading quotation mark
            if (line.startsWith(mapper.getSnapshotUncPath())) {
                line.remove(0, mapper.getSnapshotUncPath().size());
                line.prepend(ITEM_NAME_PREFIX + mapper.getPartition() + ":");
                aclMetadataFromLocalPath.append(line);
           }
        }
    }
    if (!origMetadataFile.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Can not write to file " << origMetadataFile.fileName();
        return;
    } else {
        QTextStream outStream(&origMetadataFile);
        outStream.setCodec("UTF-16");
        foreach(QString item, aclMetadataFromLocalPath) {
            outStream << item << '\n';
        }
        origMetadataFile.close();
    }
}
