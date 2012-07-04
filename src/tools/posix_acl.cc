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

#include "settings/settings.hh"
#include "settings/platform.hh"
#include "test/test_manager.hh"
#include "tools/abstract_rsync.hh"
#include "tools/posix_acl.hh"
#include "utils/unicode_text_stream.hh"
#include "utils/log_file_utils.hh"
#include "utils/file_system_utils.hh"

#include <unistd.h>

const QString PosixAcl::ITEM_NAME_PREFIX = "# file: /";

PosixAcl::PosixAcl() :
    getfaclName(Settings::getInstance()->getGetfaclName()),
    setfaclName(Settings::getInstance()->getGetfaclName())
{
}

PosixAcl::PosixAcl(const QString& getfacl, const QString& setfacl) :
    getfaclName(getfacl),
    setfaclName(setfacl)
{
}

PosixAcl::~PosixAcl()
{
}

QString PosixAcl::getMetadata(
        const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems,
        QString* warnings)
{
    Settings* settings(Settings::getInstance());
    return getMetadata(
            settings->getApplicationDataDir() + settings->getTempMetadataFileName(),
            processedItems,
            warnings);
}

QString PosixAcl::getMetadata(
        const QString& aclFileName,
        const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems,
        QString* warnings)
{
	qDebug() << "PosixAcl::getMetadata( processedItems )";
	QStringList arguments;
	arguments << "--absolute-names";
	arguments << "--physical";
	arguments << "-";
	createProcess(this->getfaclName, arguments);
	setStandardOutputFile(aclFileName);
	start();

	waitForStarted();
	for (int i=0; i<processedItems.size(); i++)
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> processedItem = processedItems.at(i);
		if (processedItem.second != AbstractRsync::DELETED)
		{
			//qDebug() << "Getting acl for item " << processedItem.first;
			//LogFileUtils::getInstance()->logToHex(processedItem.first);
			//LogFileUtils::getInstance()->logToHex(processedItem.first.toLocal8Bit());
			emit infoSignal(QObject::tr( "Getting metadata for %1" ).arg(processedItem.first) );
			write(processedItem.first.toLocal8Bit().data() );
			write(Platform::EOL_CHARACTER);
		}
	}

	closeWriteChannel();
	waitForFinished();

	try
	{
		FileSystemUtils::convertFile(aclFileName, "", "utf-8");
	}
	catch ( const ProcessException& e )
	{
		qWarning() << QString( e.what() );
	}

	if (warnings)
	{
		*warnings = readAllStandardError();
		if (!warnings->isEmpty())
		{
			warnings->append(tr("\nWarning: The permissions of some files could not be backed up. See above for details."));
		}
	}

	return aclFileName;
}

void PosixAcl::mergeMetadata(const QFileInfo& newMetadataFileName, const QFileInfo& currentMetadataFileName,
	const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems)
{
	qDebug() << "PosixAcl::mergeMetadata( " << newMetadataFileName.absoluteFilePath() << ", "
			<< currentMetadataFileName.absoluteFilePath() << ")";

	QMap<QString, QStringList> aclMap; // key: file or dir name, value: acl data (each line as a string in the list)

	// read all acl's from the current acl file and put them in a map
	populateMapFromFile(currentMetadataFileName, &aclMap);

	// delete all acl's for deleted items
	for (int i=0; i<processedItems.size(); i++)
	{
		QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> processedItem = processedItems.at(i);
		if (processedItem.second == AbstractRsync::DELETED)
		{
			aclMap.remove(processedItem.first);
		}
	}

	// read all acl's from the new acl file and put them in the map. if there is already an entry, replace it
	populateMapFromFile(newMetadataFileName, &aclMap);

	writeMapContentToFile(aclMap, currentMetadataFileName);
}

void PosixAcl::setMetadata(const QFileInfo& metadataFileName, const QStringList& downloadedItems,
	const QString& downloadDestination)
{
	qDebug() << "PosixAcl::setMetadata( " << metadataFileName.absoluteFilePath() << ", " << downloadedItems << ", "
			<< downloadDestination << " )";

	QMap<QString, QStringList> aclMap; // key: file or dir name, value: acl data (each line as a string in the list)

	populateMapFromFile(metadataFileName, &aclMap);

	// remove items that do not match the downloaded item list
	foreach ( QString aclKey, aclMap.keys() )
	{
		QStringList aclValues = aclMap.value( aclKey );

		QString firstLine = aclValues.at( 0 );
		QString itemName = firstLine.mid( ITEM_NAME_PREFIX.size() );

		bool ok;
		itemName = unescapeOctalCharacters( itemName, &ok );
		if ( !ok )
		{
			aclMap.remove( aclKey );
			emit errorSignal( QObject::tr( "Can not set metadata for %1 because the metadata are broken" ).arg( itemName ) );
			continue;
		}

		qDebug() << "ACLs found for " << itemName;
		if ( !downloadedItems.contains( itemName ) )
		{
			qDebug() << "Removing ACL for: " << aclKey;
			aclMap.remove( aclKey );
		}
		else
		{
			// prefix destination path
			QString absoluteItemPath;
			if ( downloadDestination != "/" )
			{
				absoluteItemPath = downloadDestination + "/" + itemName;
			}
			else
			{
				absoluteItemPath = downloadDestination + itemName;
			}
			firstLine = ITEM_NAME_PREFIX.left( ITEM_NAME_PREFIX.size() -1 ) + absoluteItemPath;
			aclValues.replace( 0, firstLine );
			aclMap.remove( itemName ); // remove old
			aclMap.insert( absoluteItemPath, aclValues ); // insert new
			qDebug() << "Setting acl for: " << absoluteItemPath;
			emit infoSignal( QObject::tr( "Setting metadata for %1" ).arg( absoluteItemPath ) );
		}
	}

	writeMapContentToFile(aclMap, metadataFileName);
	try
	{
		FileSystemUtils::convertFile(metadataFileName.absoluteFilePath(), "utf-8", "");
	}
	catch ( const ProcessException& e )
	{
		qWarning() << QString( e.what() );
	}

	// apply acl
	QStringList arguments;
	arguments << "--restore=" + metadataFileName.absoluteFilePath();
	createProcess(setfaclName, arguments);
	start();
	waitForFinished();
	QString errors = readAllStandardError();
	if (errors != "")
	{
		qDebug() << "in SetAcl::setMetadata" << "filtering errors:" << errors;
		QStringList errList = errors.split("\n");
		for (int i = 0; i < errList.size();) {
			QString errStr = errList.at(i);
			if ( (geteuid() != 0 && errStr.contains("setfacl:")  && errStr.contains("Cannot change owner/group: Operation not permitted")) || errStr == "" ) {
				errList.removeAt(i);
			} else {
				i++;
			}
		}
		if (errList.size() > 0) {
			errors = errList.join("\n");
			qWarning() << "Error occurred while setting ACL's: " << errors;
			throw ProcessException( QObject::tr( "Error occurred while setting ACL's:\n" ) + errors );
		}
	}
}

QStringList PosixAcl::extractItems(const QFileInfo& metadataFile)
{
	qDebug() << "PosixAcl::extractItems( " << metadataFile.absoluteFilePath() << " )";

	QMap<QString, QStringList> aclMap; // key: file or dir name, value: acl data (each line as a string in the list)
	populateMapFromFile(metadataFile, &aclMap);
	return aclMap.keys();
}

void PosixAcl::populateMapFromFile(const QFileInfo& aclFileName, QMap<QString, QStringList>* aclMap)
{
	qDebug() << "PosixAcl::populateMapFromFile( " << aclFileName.absoluteFilePath() << ", aclMap )";

	QFile aclFile(aclFileName.absoluteFilePath() );
	if ( !aclFile.open(QIODevice::ReadOnly) )
	{
		qWarning() << "Can not read from file " << aclFileName.absoluteFilePath();
		return;
	}
	else
	{
		qDebug() << "Parsing file " << aclFileName.absoluteFilePath();
	}

	UnicodeTextStream textStream( &aclFile);
	while ( !textStream.atEnd() )
	{
		QString line = textStream.readLine();
		if ( !line.startsWith(ITEM_NAME_PREFIX) )
		{
			qCritical() << "Unsupported ACL file format";
			return;
		}

		// extract file or directory name
		QString itemName = line.mid(ITEM_NAME_PREFIX.size() );

		QStringList aclData(line);

		while (line != "")
		{
			line = textStream.readLine();
			aclData << line;
		}

		aclMap->insert(itemName, aclData); // overrides the element if it already exists
	}
	aclFile.close();
}

void PosixAcl::writeMapContentToFile(const QMap<QString, QStringList>& aclMap, const QFileInfo& aclFileName)
{
	qDebug() << "PosixAcl::writeMapContentToFile( aclMap, " << aclFileName.absoluteFilePath() << " )";

	// if the file already exists, delete and recreate the acl file, then write the content of the map to the file
	QFile aclFile(aclFileName.absoluteFilePath() );
	if (aclFile.exists() )
	{
		aclFile.remove();
	}
	if ( !aclFile.open(QIODevice::WriteOnly) )
	{
		qWarning() << "Can not write to file " << aclFileName.absoluteFilePath();
		return;
	}
	UnicodeTextStream textStream( &aclFile);
	QList<QStringList> aclMapValues = aclMap.values();
	foreach ( QStringList aclValues, aclMapValues )
	{
		foreach ( QString aclValue, aclValues )
		{
			textStream << aclValue << endl;
		}
	}
	aclFile.close();
}

QString PosixAcl::unescapeOctalCharacters(const QString& escapedString, bool* ok)
{
	qDebug() << "unescapeOctalCharacters( " << escapedString << " )";
	if ( !escapedString.contains('\\'))
	{
		*ok = true;
		return escapedString;
	}
	QVector<char> unescapedCharacters;
	for (int i=0; i<escapedString.size(); i++)
	{
		char character = escapedString.at( i ).toLatin1(); //correct?
		if (character == '\\')
		{
			QString escapedCharacter = escapedString.mid( ++i, 3);

			bool toIntOk;
			int unescapedCharacter = escapedCharacter.toInt( &toIntOk, 8);
			if ( !toIntOk)
			{
				qWarning() << "Unsupported character found in meta data";
				*ok = false;
				return escapedString;
			}

			unescapedCharacters << unescapedCharacter;
			i+=2;
		}
		else
		{
			unescapedCharacters << character;
		}
	}
	QString result = QString::fromLocal8Bit(unescapedCharacters.data(), unescapedCharacters.size() );
	qDebug() << "unescaped : " << result;
	*ok = true;
	return result;
}

void PosixAcl::testUnescapeOctalCharacters()
{
	bool ok;
	qDebug() << unescapeOctalCharacters("/tmp2/uml\\303\\244\\303\\266\\303\\274.txt", &ok);
	qDebug() << unescapeOctalCharacters("tmp/permtest/sub1/2.txt", &ok);
}

void PosixAcl::testGetMetadata()
{
	QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> > processedItems;
	processedItems << qMakePair(QString("/tmp2/umläöü.txt"), AbstractRsync::TRANSFERRED);
	PosixAcl posixAcl;
	posixAcl.getMetadata(processedItems);
}

void PosixAcl::testSetMetadata()
{
	Settings* settings = Settings::getInstance();
	QString aclFile = settings->getApplicationDataDir() + settings->getMetadataFileName();

	QStringList downloadedFiles;
	downloadedFiles << "/tmp2/umläöü.txt";

	PosixAcl posixAcl;
	posixAcl.setMetadata(aclFile, downloadedFiles, "/");
}

namespace
{
int dummy = TestManager::registerTest("posixAcl_testGetMetadata", PosixAcl::testGetMetadata);
int dummy2 = TestManager::registerTest("posixAcl_testSetMetadata", PosixAcl::testSetMetadata);
int dummy3 = TestManager::registerTest("testUnescapeOctalCharacters", PosixAcl::testUnescapeOctalCharacters);
}
