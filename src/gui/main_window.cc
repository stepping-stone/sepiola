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
#include <QMessageBox>

#include "gui/about_dialog.hh"
#include "gui/logfile_form.hh"
#include "gui/main_window.hh"
#include "gui/password_dialog.hh"

MainWindow::MainWindow ( MainModel *model ) : QMainWindow()
{
	setupUi ( this );
	Settings* settings = Settings::getInstance();
	resize( settings->getWindowSize() );
	move( settings->getWindowPosition() );
	this->setWindowTitle(settings->getApplicationName());
	this->model = model;

	QObject::connect( this->model, SIGNAL( showInformationMessageBox( QString ) ),
						this, SLOT( showInformationMessageBox( QString ) ) );
	QObject::connect( this->model, SIGNAL( showCriticalMessageBox( QString ) ),
						this, SLOT( showCriticalMessageBox( QString ) ) );
	QObject::connect( this->model, SIGNAL( askForPassword( const QString&, bool, int*, const QString& ) ),
						this, SLOT( showPasswordDialog( const QString&, bool, int*, const QString& ) ) );
	QObject::connect( this->model, SIGNAL( showProgressDialog( const QString& ) ),
						this, SLOT( showProgressDialog( const QString& ) ) );
	QObject::connect( this->model, SIGNAL( appendInfoMessage( const QString& ) ),
						this, SLOT( appendInfoMessage( const QString& ) ) );
	QObject::connect( this->model, SIGNAL( appendErrorMessage( const QString& ) ),
						this, SLOT( appendErrorMessage( const QString& ) ) );
	QObject::connect( this->model, SIGNAL( finishProgressDialog() ),
						this, SLOT( finishProgressDialog() ) );
	QObject::connect( this->model, SIGNAL( closeProgressDialog() ),
						this, SLOT( closeProgressDialog() ) );

	// log file signals/slots
	QObject::connect( this->model, SIGNAL( appendInfoMessage( const QString& ) ),
							this, SIGNAL( writeLog( const QString& ) ) );
	QObject::connect( this->model, SIGNAL( appendErrorMessage( const QString& ) ),
							this, SIGNAL( writeLog( const QString& ) ) );

	if ( settings->isReinstalled() )
	{
		int answer = QMessageBox::question ( this, tr ( "Keep settings?" ),
																				 tr ( "The application has been reinstalled.\n"
																							"Do you want to keep the settings?" ),
																				 QMessageBox::Yes | QMessageBox::No );
		switch ( answer )
		{
			case QMessageBox::Yes:
				model->keepSettings();
				break;
			case QMessageBox::No:
				model->deleteSettings();
				break;
			default:
				// should never be reached
				break;
		}
	}
	this->backupForm = new BackupForm ( 0, this->model );
	this->restoreForm = new RestoreForm ( 0, this->model );
	this->settingsForm = new SettingsForm ( 0, this->model );
	this->logfileForm = new LogfileForm( 0, this->model );

	stackedLayout = new QStackedLayout( frameMain );
	stackedLayout->addWidget( backupForm );
	stackedLayout->addWidget( restoreForm );
	stackedLayout->addWidget( settingsForm );
	stackedLayout->addWidget( logfileForm );
	frameMain->setLayout ( stackedLayout );
	MainWindow::setFormIndex( SETTINGS );
}

MainWindow::~MainWindow()
{
}

void MainWindow::showPasswordDialog( const QString& username, bool isUsernameEditable, int* result, const QString& msg )
{
	PasswordDialog passwordDialog( username, isUsernameEditable );
	QObject::connect( &passwordDialog, SIGNAL( abort() ),
						this->model, SLOT( abortLogin() ) );
	if (msg != "")
		passwordDialog.setDialogMessage(msg);
    int dialog_result = passwordDialog.exec(); 
	if (result) 
	{
		*result = dialog_result; 
	}
	QObject::disconnect( &passwordDialog, SIGNAL( abort() ),
							 this->model, SIGNAL( abortLogin() ) );
}

void MainWindow::showInformationMessageBox( const QString& message )
{
	Settings* settings = Settings::getInstance();
	QMessageBox::information( 0, settings->getApplicationName(), message );
}

void MainWindow::showCriticalMessageBox( const QString& message )
{
	Settings* settings = Settings::getInstance();
	QMessageBox::critical( 0, settings->getApplicationName(), message );
}

void MainWindow::setFormIndex( FORM_INDEX index )
{
	switch( index )
	{
		case SETTINGS:
			this->settingsForm->reload();
			break;
		default:
			// do nothing
			break;
	}
	this->stackedLayout->setCurrentIndex( index );
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

void MainWindow::showBackupForm()
{
	MainWindow::setFormIndex ( BACKUP );
}

void MainWindow::showRestoreForm()
{
	MainWindow::setFormIndex ( RESTORE );
	if( !this->restoreForm->isInitialized() )
	{
		this->restoreForm->initPrefixes();
	}
}

void MainWindow::showSettingsForm()
{
	MainWindow::setFormIndex ( SETTINGS );
}

void MainWindow::showLogfileForm()
{
	MainWindow::setFormIndex( LOGFILE );
	QStringList lines = this->model->getNewLogfileLines();
	this->logfileForm->appendLines( lines );
}


void MainWindow::on_actionAbout_triggered()
{
	AboutDialog aboutDialog;
	aboutDialog.exec();
}

void MainWindow::closeEvent ( QCloseEvent *event )
{
	Settings* settings = Settings::getInstance();
	settings->saveWindowSize( size() );
	settings->saveWindowPosition( pos() );

	this->model->exit();
	event->accept();
}

void MainWindow::showProgressDialog( const QString&  title )
{
	outputDialog = new OutputDialog( title );
	QObject::connect( outputDialog, SIGNAL( abort() ),
						this->model, SLOT( abortProcessSlot() ) );
	outputDialog->show();
	QApplication::processEvents();
}

void MainWindow::appendInfoMessage( const QString& info )
{
	outputDialog->appendInfo( info );
}

void MainWindow::appendErrorMessage( const QString& error )
{
	outputDialog->appendError( error );
}

void MainWindow::finishProgressDialog()
{
	outputDialog->finished();
	QApplication::processEvents();
}

void MainWindow::closeProgressDialog()
{
	outputDialog->done(0);
	QApplication::processEvents();
}

