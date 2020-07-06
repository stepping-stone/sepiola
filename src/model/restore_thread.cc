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

#include "model/restore_thread.hh"
#include "exception/abort_exception.hh"
#include "exception/process_exception.hh"
#include "model/main_model.hh"
#include "settings/settings.hh"
#include "tools/abstract_metadata.hh"
#include "tools/tool_factory.hh"
#include "utils/file_system_utils.hh"

#include <memory>

RestoreThread::RestoreThread(const QString &backup_prefix,
                             const QString &backupName,
                             const BackupSelectionHash &selectionRules,
                             const QString &destination)
{
    this->isCustomRestore = true;
    this->backup_prefix = backup_prefix;
    this->backupName = backupName;
    this->selectionRules = selectionRules;
    this->destination = destination;
    this->restoreState = ConstUtils::STATUS_UNDEFINED;
    init();
}

RestoreThread::~RestoreThread()
{
    qDebug() << "RestoreThread::~RestoreThread()";
    QObject::disconnect(rsync.get(),
                        SIGNAL(infoSignal(const QString &)),
                        this,
                        SIGNAL(infoSignal(const QString &)));
    QObject::disconnect(rsync.get(),
                        SIGNAL(errorSignal(const QString &)),
                        this,
                        SIGNAL(errorSignal(const QString &)));
    QObject::disconnect(this, SIGNAL(abort()), rsync.get(), SLOT(abort()));
    // QObject::disconnect(rsync.get(),
    //                     SIGNAL(trafficInfoSignal(const QString &, float, quint64, quint64)),
    //                     this,
    //                     SLOT(rsyncUploadProgressHandler(const QString &, float, quint64, quint64)));
}

void RestoreThread::init()
{
    isAborted = false;
    rsync.reset(ToolFactory::getRsyncImpl());
    QObject::connect(rsync.get(),
                     SIGNAL(infoSignal(const QString &)),
                     this,
                     SIGNAL(infoSignal(const QString &)));
    QObject::connect(rsync.get(),
                     SIGNAL(errorSignal(const QString &)),
                     this,
                     SIGNAL(infoSignal(const QString &)));
    QObject::connect(this, SIGNAL(abort()), rsync.get(), SLOT(abort()));
    // QObject::connect(rsync.get(),
    //                  SIGNAL(trafficInfoSignal(const QString &, float, quint64, quint64)),
    //                  this,
    //                  SLOT(rsyncUploadProgressHandler(const QString &, float, quint64, quint64)));
    // TODO: this is copied from BackupThread -> adapt as needed, as well as the above disconnection
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void RestoreThread::pushStateEvent(ConstUtils::StatusEnum eventState)
{
    // if necessary increases this->restoreState to eventState (no lowering of state is done: state
    // cannot get "better", f.ex. from ERROR to OK)
    this->restoreState = (ConstUtils::StatusEnum) std::max((int) eventState,
                                                           (int) (this->restoreState));
}

void RestoreThread::run()
{
    try {
        emit infoSignal(tr("Downloading files and/or directories ..."));
        QStringList downloadedItems;
        if (isCustomRestore) {
            downloadedItems = rsync->downloadCustomBackup(backup_prefix,
                                                          backupName,
                                                          selectionRules,
                                                          destination);
        } else {
            downloadedItems = rsync->downloadFullBackup(backup_prefix, backupName, destination);
        }
        checkAbortState();
        emit infoSignal(tr("Applying Metadata"));
        applyMetadata(backup_prefix, backupName, downloadedItems, destination);
        checkAbortState();
        this->pushStateEvent(ConstUtils::STATUS_OK);
        emit infoSignal(tr("Restore done."));
    } catch (const ProcessException &e) {
        emit infoSignal(tr("Restore failed."));
        emit errorSignal(e.what());
        this->pushStateEvent(ConstUtils::STATUS_ERROR);
    } catch (const AbortException &e) {
        emit infoSignal(tr("Restore aborted."));
        emit errorSignal(e.what());
        this->pushStateEvent(ConstUtils::STATUS_ERROR);
    }
    emit finalStatusSignal(this->restoreState);
    emit finishProgressDialog();
}

void RestoreThread::applyMetadata(const QString &backup_prefix,
                                  const QString &backupName,
                                  const QStringList &downloadedItems,
                                  const QString &downloadDestination)
{
    if (downloadedItems.size() > 0) {
        std::shared_ptr<AbstractMetadata> metadata(ToolFactory::getMetadataImpl());
        QObject::connect(metadata.get(),
                         SIGNAL(infoSignal(const QString &)),
                         this,
                         SIGNAL(infoSignal(const QString &)));
        QObject::connect(metadata.get(),
                         SIGNAL(errorSignal(const QString &)),
                         this,
                         SIGNAL(errorSignal(const QString &)));
        QFileInfo metadataFile
            = rsync->downloadMetadata(backup_prefix,
                                      backupName,
                                      Settings::getInstance()->getApplicationDataDir());
        if (metadataFile.fileName() != "" && metadataFile.exists()) {
            metadata->setMetadata(metadataFile, downloadedItems, downloadDestination);
        } else {
            QString msg = tr("WARNING: Setting of metadata failed. File %1 has not been found on "
                             "the server. Possibly you are restoring data from a backup of another "
                             "platform/os.")
                              .arg(Settings::getInstance()->getMetadataFileName());
            qWarning() << msg;
            emit infoSignal(msg);
            this->pushStateEvent(ConstUtils::STATUS_WARNING);
        }
        FileSystemUtils::removeFile(metadataFile);
        QObject::disconnect(metadata.get(),
                            SIGNAL(infoSignal(const QString &)),
                            this,
                            SIGNAL(infoSignal(const QString &)));
        QObject::disconnect(metadata.get(),
                            SIGNAL(errorSignal(const QString &)),
                            this,
                            SIGNAL(infoSignal(const QString &)));
    }
}

void RestoreThread::checkAbortState()
{
    if (isRunning() && isAborted) {
        throw AbortException(tr("Restore has been aborted"));
    }
}

void RestoreThread::abortRestoreProcess()
{
    qDebug() << "RestoreThread::abortRestoreProcess()";
    isAborted = true;
    emit abort();
}
