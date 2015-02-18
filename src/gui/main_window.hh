/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2011 stepping stone GmbH
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

#ifndef MAIN_WINDOW_HH
#define MAIN_WINDOW_HH

#include <QMainWindow>
#include <QCloseEvent>

#include "ui_main_window.h"

class MainModel;
class OverviewForm;
class BackupForm;
class SettingsForm;
class RestoreForm;
class LogfileForm;
class TrafficProgressDialog;
class QStackedLayout;

/**
 * The MainWindow class is the main user interface.
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	/**
	 * Constructs a MainWindow with a given model
	 */
	MainWindow ( MainModel *model );

	/**
	 * Destroys the MainWindow
	 */
	virtual ~MainWindow();
	void show();

signals:
	void rejectPasswordDialog();
	void writeLog( const QString& message );

	void updateOverviewFormScheduleInfo();
	void updateOverviewFormLastBackupsInfo();

private:
	enum FORM_INDEX { OVERVIEW, BACKUP, RESTORE, SETTINGS, LOGFILE };
	QStackedLayout *stackedLayout;
	MainModel* model;
	OverviewForm* overviewForm;
	BackupForm* backupForm;
	RestoreForm* restoreForm;
	SettingsForm* settingsForm;
	LogfileForm* logfileForm;
	TrafficProgressDialog* progressDialog;

	void writeSettings();
	void readSettings();
	void closeEvent ( QCloseEvent *event );
	void setFormIndex( FORM_INDEX index );
	void showOverviewForm();
	void showBackupForm();
	void showRestoreForm();
	void showSettingsForm();
	void showLogfileForm();

private slots:
	void showInformationMessageBox( const QString& message );
	void showCriticalMessageBox( const QString& message );
	void showServerPasswordDialog( const QString& username, bool isUsernameEditable, int* result = 0, const QString& msg = "" );
	void showClientPasswordDialog( const QString& username, bool isUsernameEditable, int* result = 0, const QString& msg = "" );
	
	void showProgressDialog( const QString& dialogTitle );
	void finishProgressDialog();
	void closeProgressDialog();
	
	void on_actionOverview_triggered();
	void on_actionBackup_triggered();
	void on_actionRestore_triggered();
	void on_actionSettings_triggered();
	void on_actionLogfile_triggered();
	void on_actionAbout_triggered();
	void on_btnOverview_clicked();
	void on_btnBackup_clicked();
	void on_btnRestore_clicked();
	void on_btnSettings_clicked();
	void on_btnLogfile_clicked();
};

#endif
