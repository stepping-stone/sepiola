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

#ifndef SET_ACL_HH
#define SET_ACL_HH

#include <QString>

#include "abstract_metadata.hh"
#include "process.hh"

/**
 * The SetAcl class provides methods for using the setacl tool
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class SetAcl : public AbstractMetadata, public Process
{
public:
    /**
     * Constructs a SetAcl
     */
    SetAcl(const QString &setAcl);

    /**
     * Destroys the SetAcl
     */
    virtual ~SetAcl();

    /**
     * @see AbstractMetadata::getMetadata( const QList< QPair<QString,
     * AbstractRsync::ITEMIZE_CHANGE_TYPE> >& processedItems )
     */
    QString getMetadata(
        const QString &aclFileName,
        const QList<QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>> &processedItems,
        const FilesystemSnapshot *fsSnapshot,
        QString *warnings = 0);

    /**
     * @see AbstractMetadata::mergeMetadata( const QFileInfo& newMetadataFileName, const QFileInfo&
     * currentMetadataFileName, const QList< QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE> >&
     * processedItems )
     */
    void mergeMetadata(
        const QFileInfo &newMetadataFileName,
        const QFileInfo &currentMetadataFileName,
        const QList<QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>> &processedItems);

    /**
     * @see AbstractMetadata::setMetadata( const QFileInfo& metadataFile, const QStringList&
     * downloadedItems, const QString& downloadDestination )
     */
    void setMetadata(const QFileInfo &metadataFile,
                     const QStringList &downloadedItems,
                     const QString &downloadDestination);

    /**
     * @see AbstractMetadata::extractItems( const QFileInfo& metadataFile )
     */
    QStringList extractItems(const QFileInfo &metadataFile);

private:
    static const QString ITEM_NAME_PREFIX;
    static const uint ITEM_NAME_PREFIX_SIZE;
    const QString setAclName;

    void populateMapFromFile(const QFileInfo &aclFileName, QMap<QString, QString> *aclMap);
    void writeMapContentToFile(const QMap<QString, QString> &aclMap, const QFileInfo &aclFileName);
    QString extractItemNameFromAclValue(const QString aclValue);
    void replaceUncPathsByLocalPathsInMetadataFile(const QFileInfo &Filename,
                                                   const FilesystemSnapshot *snapshot) const;
};

#endif
