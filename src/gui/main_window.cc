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
#include <QMessageBox>
#include <QStackedLayout>

#include "gui/about_dialog.hh"
#include "gui/backup_form.hh"
#include "gui/logfile_form.hh"
#include "gui/main_window.hh"
#include "gui/overview_form.hh"
#include "gui/password_dialog.hh"
#include "gui/restore_form.hh"
#include "gui/settings_form.hh"
#include "gui/traffic_progress_dialog.hh"
#include "model/main_model.hh"
#include "model/space_usage_model.hh"
#include "settings/settings.hh"

MainWindow::MainWindow(MainModel *model)
    : QMainWindow()
{
    setupUi(this);
    this->progressDialog = 0;
    Settings *settings = Settings::getInstance();
    resize(settings->getWindowSize());
    move(settings->getWindowPosition());
    this->setWindowTitle(settings->getApplicationName());
    this->model = model;

    QObject::connect(this->model,
                     SIGNAL(showInformationMessageBox(QString)),
                     this,
                     SLOT(showInformationMessageBox(QString)));
    QObject::connect(this->model,
                     SIGNAL(showCriticalMessageBox(QString)),
                     this,
                     SLOT(showCriticalMessageBox(QString)));
    QObject::connect(this->model,
                     SIGNAL(askForServerPassword(const QString &, bool, int *, const QString &)),
                     this,
                     SLOT(showServerPasswordDialog(const QString &, bool, int *, const QString &)));
    QObject::connect(this->model,
                     SIGNAL(askForClientPassword(const QString &, bool, int *, const QString &)),
                     this,
                     SLOT(showClientPasswordDialog(const QString &, bool, int *, const QString &)));
    QObject::connect(this->model,
                     SIGNAL(showProgressDialog(const QString &)),
                     this,
                     SLOT(showProgressDialog(const QString &)));

    if (settings->isReinstalled()) {
        int answer = QMessageBox::question(
            this,
            tr("Keep settings?"),
            tr("The application has been reinstalled.\nDo you want to keep the settings?"),
            QMessageBox::Yes | QMessageBox::No);
        switch (answer) {
        case QMessageBox::No:
            model->deleteSettings();
            break;
        default:
            model->keepSettings();
            break;
        }
    }
    this->overviewForm = new OverviewForm(this, this->model);
    this->backupForm = new BackupForm(this, this->model);
    this->restoreForm = new RestoreForm(this, this->model);
    this->settingsForm = new SettingsForm(this, this->model);
    this->logfileForm = new LogfileForm(this, this->model);

    QObject::connect(this,
                     SIGNAL(updateOverviewFormScheduleInfo()),
                     overviewForm,
                     SLOT(refreshScheduleOverview()));
    QObject::connect(model,
                     SIGNAL(updateOverviewFormLastBackupsInfo()),
                     overviewForm,
                     SLOT(refreshLastBackupsOverview()));
    QObject::connect(this,
                     SIGNAL(updateOverviewFormLastBackupsInfo()),
                     overviewForm,
                     SLOT(refreshLastBackupsOverview()));
    QObject::connect(this->overviewForm,
                     SIGNAL(startBackupNow()),
                     this->backupForm,
                     SLOT(runBackupNow()));
    QObject::connect(this->settingsForm,
                     SIGNAL(showHiddenFilesAndFolders(bool)),
                     this->backupForm,
                     SLOT(showHiddenFilesAndFolders(bool)));

    stackedLayout = new QStackedLayout(frameMain);
    stackedLayout->addWidget(overviewForm);
    stackedLayout->addWidget(backupForm);
    stackedLayout->addWidget(restoreForm);
    stackedLayout->addWidget(settingsForm);
    stackedLayout->addWidget(logfileForm);
    frameMain->setLayout(stackedLayout);
    MainWindow::setFormIndex(OVERVIEW);
}

MainWindow::~MainWindow() {}

void MainWindow::show()
{
    QWidget::show();
    Settings *settings = Settings::getInstance();
    if (settings->isInevitableSettingsMissing()) {
        MainWindow::setFormIndex(SETTINGS);
        QMessageBox::information(this,
                                 tr("Missing Settings"),
                                 tr("Please fill in username (as provided by the backup-space "
                                    "provider) as well as the computername."));
    } else {
        MainWindow::setFormIndex(OVERVIEW);
    }
}

void MainWindow::showServerPasswordDialog(const QString &username,
                                          bool isUsernameEditable,
                                          int *result,
                                          const QString &msg)
{
    Settings *settings = Settings::getInstance();
    PasswordDialog passwordDialog(username, isUsernameEditable);
    QObject::connect(&passwordDialog, SIGNAL(abort()), this->model, SLOT(abortLogin()));
    QObject::connect(&passwordDialog,
                     SIGNAL(processPasswordReturnValues(const QString &, const QString &, bool)),
                     settings,
                     SLOT(setServerUserNameAndPassword(const QString &, const QString &, bool)));
    if (msg != "")
        passwordDialog.setDialogMessage(msg);
    int dialog_result = passwordDialog.exec();
    if (result) {
        *result = dialog_result;
    }
    QObject::disconnect(&passwordDialog, SIGNAL(abort()), this->model, SIGNAL(abortLogin()));
    QObject::disconnect(&passwordDialog,
                        SIGNAL(processPasswordReturnValues(const QString &, const QString &, bool)),
                        settings,
                        SLOT(setServerUserNameAndPassword(const QString &, const QString &, bool)));
}

void MainWindow::showClientPasswordDialog(const QString &username,
                                          bool isUsernameEditable,
                                          int *result,
                                          const QString &msg)
{
    Settings *settings = Settings::getInstance();
    PasswordDialog passwordDialog(username, isUsernameEditable);
    QObject::connect(&passwordDialog, SIGNAL(abort()), this->model, SLOT(abortLogin()));
    QObject::connect(&passwordDialog,
                     SIGNAL(processPasswordReturnValues(const QString &, const QString &, bool)),
                     settings,
                     SLOT(setClientUserNameAndPassword(const QString &, const QString &, bool)));
    if (msg != "")
        passwordDialog.setDialogMessage(msg);
    int dialog_result = passwordDialog.exec();
    if (result) {
        *result = dialog_result;
    }
    QObject::disconnect(&passwordDialog, SIGNAL(abort()), this->model, SIGNAL(abortLogin()));
    QObject::disconnect(&passwordDialog,
                        SIGNAL(processPasswordReturnValues(const QString &, const QString &, bool)),
                        settings,
                        SLOT(setClientUserNameAndPassword(const QString &, const QString &, bool)));
}

void MainWindow::showInformationMessageBox(const QString &message)
{
    Settings *settings = Settings::getInstance();
    QMessageBox::information(this, settings->getApplicationName(), message);
}

void MainWindow::showCriticalMessageBox(const QString &message)
{
    Settings *settings = Settings::getInstance();
    QMessageBox::critical(this, settings->getApplicationName(), message);
}

void MainWindow::setFormIndex(FORM_INDEX index)
{
    qDebug() << "MainWindow::setFormIndex(" << index << ")";
    switch (index) {
    case SETTINGS:
        if (this->stackedLayout->currentIndex() != SETTINGS) {
            this->settingsForm->reload();
        }
        break;
    default:
        // do nothing
        break;
    }
    bool formChangeOk = true;
    if (this->stackedLayout->currentIndex() == SETTINGS) {
        formChangeOk = ((SettingsForm *) this->stackedLayout->currentWidget())->onLeave();
    }
    if (formChangeOk) {
        this->stackedLayout->setCurrentIndex(index);
    }
}

void MainWindow::on_btnOverview_clicked()
{
    showOverviewForm();
}

void MainWindow::on_btnBackup_clicked()
{
    showBackupForm();
}

void MainWindow::on_btnRestore_clicked()
{
    showRestoreForm();
}

void MainWindow::on_btnSettings_clicked()
{
    showSettingsForm();
}

void MainWindow::on_btnLogfile_clicked()
{
    showLogfileForm();
}

void MainWindow::on_actionOverview_triggered()
{
    showOverviewForm();
}

void MainWindow::on_actionBackup_triggered()
{
    showBackupForm();
}

void MainWindow::on_actionRestore_triggered()
{
    showRestoreForm();
}

void MainWindow::on_actionSettings_triggered()
{
    showSettingsForm();
}

void MainWindow::on_actionLogfile_triggered()
{
    showLogfileForm();
}

void MainWindow::showOverviewForm()
{
    MainWindow::setFormIndex(OVERVIEW);
}

void MainWindow::showBackupForm()
{
    MainWindow::setFormIndex(BACKUP);
}

void MainWindow::showRestoreForm()
{
    MainWindow::setFormIndex(RESTORE);
    if (!this->restoreForm->isInitialized()) {
        this->restoreForm->initPrefixes();
    }
}

void MainWindow::showSettingsForm()
{
    MainWindow::setFormIndex(SETTINGS);
}

void MainWindow::showLogfileForm()
{
    MainWindow::setFormIndex(LOGFILE);
    QStringList lines = this->model->getNewLogfileLines();
    this->logfileForm->appendLines(lines);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog aboutDialog;
    aboutDialog.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Settings *settings = Settings::getInstance();
    settings->saveWindowSize(size());
    settings->saveWindowPosition(pos());

    this->model->exit();
    event->accept();
}

void MainWindow::showProgressDialog(const QString &title)
{
    qDebug() << "MainWindow::showProgressDialog(" << title << ")";
    progressDialog = new TrafficProgressDialog(title);
    QObject::connect(progressDialog, SIGNAL(abort()), this->model, SLOT(abortProcessSlot()));
    QObject::connect(this->model,
                     SIGNAL(infoSignal(const QString &)),
                     progressDialog,
                     SLOT(appendInfo(const QString &)));
    QObject::connect(this->model,
                     SIGNAL(errorSignal(const QString &)),
                     progressDialog,
                     SLOT(appendError(const QString &)));
    QObject::connect(this->model,
                     SIGNAL(finishProgressDialog()),
                     this,
                     SLOT(finishProgressDialog()));
    QObject::connect(this->model, SIGNAL(closeProgressDialog()), this, SLOT(closeProgressDialog()));

    // log file signals/slots
    qRegisterMetaType<StringPairList>("StringPairList");
    qRegisterMetaType<ConstUtils::StatusEnum>("ConstUtils::StatusEnum");
    QObject::connect(this->model,
                     SIGNAL(
                         progressSignal(const QString &, float, const QDateTime &, StringPairList)),
                     progressDialog,
                     SLOT(
                         updateProgress(const QString &, float, const QDateTime &, StringPairList)));
    QObject::connect(this->model,
                     SIGNAL(finalStatusSignal(ConstUtils::StatusEnum)),
                     progressDialog,
                     SLOT(showFinalStatus(ConstUtils::StatusEnum)));
    QObject::connect(this->model,
                     SIGNAL(infoSignal(const QString &)),
                     this,
                     SIGNAL(writeLog(const QString &)));
    QObject::connect(this->model,
                     SIGNAL(errorSignal(const QString &)),
                     this,
                     SIGNAL(writeLog(const QString &)));
    progressDialog->show();
    QApplication::processEvents();
}

void MainWindow::finishProgressDialog()
{
    QObject::disconnect(progressDialog, SIGNAL(abort()), this->model, SLOT(abortProcessSlot()));
    QObject::disconnect(this->model,
                        SIGNAL(infoSignal(const QString &)),
                        progressDialog,
                        SLOT(appendInfo(const QString &)));
    QObject::disconnect(this->model,
                        SIGNAL(errorSignal(const QString &)),
                        progressDialog,
                        SLOT(appendError(const QString &)));
    QObject::disconnect(this->model,
                        SIGNAL(finishProgressDialog()),
                        this,
                        SLOT(finishProgressDialog()));
    QObject::disconnect(this->model,
                        SIGNAL(closeProgressDialog()),
                        this,
                        SLOT(closeProgressDialog()));
    QObject::disconnect(
        this->model,
        SIGNAL(progressSignal(const QString &, float, const QDateTime &, StringPairList)),
        progressDialog,
        SLOT(updateProgress(const QString &, float, const QDateTime &, StringPairList)));
    QObject::disconnect(this->model,
                        SIGNAL(finalStatusSignal(ConstUtils::StatusEnum)),
                        progressDialog,
                        SLOT(showFinalStatus(ConstUtils::StatusEnum)));
    QObject::disconnect(this->model,
                        SIGNAL(infoSignal(const QString &)),
                        this,
                        SIGNAL(writeLog(const QString &)));
    QObject::disconnect(this->model,
                        SIGNAL(errorSignal(const QString &)),
                        this,
                        SIGNAL(writeLog(const QString &)));

    progressDialog->finished();
    QApplication::processEvents();
}

void MainWindow::closeProgressDialog()
{
    progressDialog->done(0);
    QApplication::processEvents();
}
