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

#ifndef ABSTRACT_RSYNC_HH
#define ABSTRACT_RSYNC_HH

#include <QFileInfo>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

#include "tools/abstract_informing_process.hh"

/**
 * The AbstractRsync class provides methods for using the rsync tool
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class AbstractRsync : public AbstractInformingProcess
{
    Q_OBJECT

public:
    /**
     * Destroys the AbstractRsync
     */
    virtual ~AbstractRsync();

    /**
     * Transfer type
     */
    enum ITEMIZE_CHANGE_TYPE { UNKNOWN, TRANSFERRED, SKIPPED, DELETED };

    /**
     * Uploads the files and directory in the given list
     * @param items list of items to upload
     * @param source source directory path
     * @param destination destination directory path
     * @param includePatternList list of include pattern
     * @param excludePatternList list of exclude pattern
     * @param setDeleteFlag delete extraneous files from destination
     * @param errors a QString pointer for saving errors
     * @return a list of transfered items
     */
    virtual QList<QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>> upload(
        const BackupSelectionHash &includeRules,
        const QString &src,
        const QString &destination,
        bool setDeleteFlag,
        bool compress,
        int bandwidthLimit,
        QString *warnings,
        bool dry_run)
        = 0;
    virtual QList<QPair<QString, AbstractRsync::ITEMIZE_CHANGE_TYPE>> upload(
        const QStringList &items,
        const QString &source,
        const QString &destination,
        const QStringList &includePatternList,
        const QStringList &excludePatternList,
        bool setDeleteFlag,
        bool compress,
        int bandwidthLimit,
        QString *warnings,
        bool dry_run)
        = 0;

    virtual long calculateUploadTransfer(const BackupSelectionHash includeRules,
                                         const QString &src,
                                         const QString &destination,
                                         bool setDeleteFlag,
                                         bool compress,
                                         int bandwidthLimit,
                                         QString *errors,
                                         QString *warnings)
        = 0;

    void upload(const QFileInfo &fileInfo,
                const QString &destination,
                bool bCompress,
                int bandwidthLimit,
                QString *warnings);

    /**
     * Downloads a full backup of the given name
     * @param backupName name of the backup
     * @param destination destination directory path
     * @return a list of downloaded items
     */
    virtual QStringList downloadFullBackup(const QString &backup_prefix,
                                           const QString &backupName,
                                           const QString &destination)
        = 0;

    /**
     * Downloads a custom list of files and dirs to a given destination
     * @param backupName name of the backup
     * @param itemList list of items to download
     * @param destination destination directory path
     * @return a list of downloaded items
     */
    virtual QStringList downloadCustomBackup(const QString &backup_prefix,
                                             const QString &backupName,
                                             const BackupSelectionHash &includeRules,
                                             const QString &destination)
        = 0;
    virtual QStringList downloadCustomBackup(const QString &backup_prefix,
                                             const QString &backupName,
                                             const QStringList &itemList,
                                             const QString &destination)
        = 0;

    /**
     * Downloads the backup content file
     * @param backupName name of the backup
     * @param destination destination directory path
     * @return the content file
     */
    virtual QFileInfo downloadBackupContentFile(const QString &backup_prefix,
                                                const QString &backupName,
                                                const QString &destination)
        = 0;

    /**
     * Downloads the current backup content file
     * @param destination destination directory path
     * @param emitErrorSignal flag to enable/disable error signal emitting
     * @return the content file
     */
    virtual QFileInfo downloadCurrentBackupContentFile(const QString &destination,
                                                       bool emitErrorSignal = true)
        = 0;

    /**
     * Downloads the metadata file for the given backup name
     * @param backupName name of the backup
     * @param destination destination directory path
     * @return the metadata file
     */
    virtual QFileInfo downloadMetadata(const QString &backup_prefix,
                                       const QString &backupName,
                                       const QString &destination)
        = 0;

    /**
     * Downloads the current metadata file
     * @param destination destination directory path
     * @param emitErrorSignal flag to enable/disable error signal emitting
     * @return the metadata file
     */
    virtual QFileInfo downloadCurrentMetadata(const QString &destination, bool emitErrorSignal) = 0;

    /**
     * Downloads all restore info files
     * @param destination destination directory path
     * @return a list of available restore info files
     */
    virtual QStringList downloadAllRestoreInfoFiles(const QString &destination,
                                                    const QString &backup_prefix)
        = 0;

    /**
     * Deletes all restore info files
     * @param path restore info files path
     */
    virtual void deleteAllRestoreInfoFiles(const QString &path) = 0;

    /**
     * Gets all available prefixes from the server
     * @return all available prefixes on the server
     */
    virtual QStringList getPrefixes() = 0;

public slots:
    virtual void abort() = 0;
};

// inline functions

inline AbstractRsync::~AbstractRsync() {}

inline void AbstractRsync::upload(const QFileInfo &fileInfo,
                                  const QString &destination,
                                  bool compress,
                                  int bandwidthLimit,
                                  QString *warnings)
{
    QStringList fileNameList;
    fileNameList << fileInfo.fileName();
    upload(fileNameList,
           fileInfo.absolutePath(),
           destination,
           QStringList(),
           QStringList(),
           false,
           compress,
           bandwidthLimit,
           warnings,
           false);
}

#endif
