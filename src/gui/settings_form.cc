/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008 stepping stone GmbH
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QDebug>

#include "gui/settings_form.hh"

SettingsForm::SettingsForm( QWidget *parent, MainModel *model ) : QWidget( parent )
{
	setupUi( this );
	this->model = model;
	// change the label of ok and cancel button to save and cancel
	QList<QAbstractButton*> buttons = this->buttonBox->buttons();
	foreach( QAbstractButton* button, buttons )
	{
		switch ( this->buttonBox->buttonRole( button ) )
		{
			case QDialogButtonBox::AcceptRole:
				button->setText( tr( "S&ave" ) );
				break;
			case QDialogButtonBox::RejectRole:
				button->setText( tr( "&Cancel" ) );
				break;
			default:
				qWarning() << "Not expected button found";
		}
	}
	Settings* settings = Settings::getInstance();
	QString uid_param = settings->getQuotaModificationUrlUidParam();
	uid_param = (uid_param == "") ? "$UID$" : uid_param;
	QList<int> quotaValues = this->model->getServerQuota();
    if (quotaValues.size() > 0)
	    this->labelQuotaInformation->setText( QObject::tr("Total size: %1 GB, free: %2 GB").arg(quotaValues[0]/1024.0, 0, 'f', 1).arg((quotaValues[0]-quotaValues[2]-quotaValues[2])/1024.0, 0, 'f', 1) );
    else
        this->labelQuotaInformation->setText( QObject::tr("Total size: %1 GB, free: %2 GB").arg("-").arg("-") );
	if (settings->getQuotaModificationUrl() != "") {
		this->labelChangeQuotaLink->setText(QObject::tr("<a href=\"%1\">Change quota</a> (opens a browser window)").arg(settings->getQuotaModificationUrl().replace(uid_param, settings->getServerUserName())));
		this->labelChangeQuotaLink->setOpenExternalLinks(true);
	} else {
		this->labelChangeQuotaLink->setText("");
		this->labelChangeQuotaLink->setVisible(false);
	}

	QObject::connect( this->buttonBox, SIGNAL( accepted() ), this, SLOT( save() ) );
	QObject::connect( this->buttonBox, SIGNAL( rejected() ), this, SLOT( reset() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormLastBackupsInfo() ), parent, SIGNAL ( updateOverviewFormLastBackupsInfo() ) );
	reload();
}


SettingsForm::~SettingsForm()
{}

void SettingsForm::reload()
{
	Settings* settings = Settings::getInstance();
	this->lineEditUsername->setText( settings->getServerUserName() );
	this->lineEditBackupPrefix->setText( settings->getBackupPrefix() );
	this->spinBoxNOfShownLastBackups->setValue( settings->getNOfLastBackups() );
	this->checkBoxShowHiddenFiles->setChecked( settings->getShowHiddenFilesAndFolders() );
	this->checkBoxKeepDeletedFiles->setChecked( !settings->getDeleteExtraneousItems() );

	this->comboBoxLanguage->clear();
    for ( auto language: settings->getAvailableLanguages() )
		this->comboBoxLanguage->addItem( language.first, language.second );

	this->comboBoxLanguage->setCurrentIndex( this->comboBoxLanguage->findData(settings->getLanguage()) );
	formChanged = false;
}

void SettingsForm::save()
{
	if ( formChanged )
	{
		Settings* settings = Settings::getInstance();
		settings->saveServerUserName( this->lineEditUsername->text() );
		settings->saveBackupPrefix( this->lineEditBackupPrefix->text() );
		settings->saveLanguage( this->comboBoxLanguage->itemData(this->comboBoxLanguage->currentIndex()).toString() );
		settings->saveNOfLastBackups( this->spinBoxNOfShownLastBackups->value() );
		settings->saveShowHiddenFilesAndFolders( this->checkBoxShowHiddenFiles->isChecked() );
		settings->saveDeleteExtraneousItems( !this->checkBoxKeepDeletedFiles->isChecked() );

		QMessageBox::information( this, tr( "Settings saved" ), tr( "Settings have been saved." ) );
		emit updateOverviewFormLastBackupsInfo();
		formChanged = false;
	}
}

void SettingsForm::reset()
{
	qDebug() << "SettingsForm::reset()";
	if ( formChanged )
	{
		int answer = QMessageBox::warning( this, tr( "Unsaved settings" ), tr( "Your settings have been modified.\n" "Do you want to restore the original settings?" ), QMessageBox::Yes | QMessageBox::No );
		switch ( answer )
		{
			case QMessageBox::Yes:
				reload();
				break;
			default:
				// do nothing
				break;
		}
	}
}

bool SettingsForm::onLeave()
{
	qDebug() << "SettingsForm::onLeave: Leaving SettingsForm";
	if ( formChanged )
	{
		int answer = QMessageBox::warning( this, tr( "Unsaved settings" ), tr( "Your settings have been modified.\n" "Do you want to save the settings before leaving this view?" ), QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save );
		switch ( answer )
		{
			case QMessageBox::Save:
				save();
				return true;
				break;
			case QMessageBox::Discard:
				return true;
				break;
			case QMessageBox::Cancel:
				return false;
				break;
			default:
				return true;
				break;
		}
	}
	return true;
}

void SettingsForm::on_lineEditUsername_textEdited( QString username )
{
	formChanged = ( username != Settings::getInstance()->getServerUserName() );
}

void SettingsForm::on_comboBoxLanguage_currentIndexChanged( int languageIndex )
{
	formChanged = ( this->comboBoxLanguage->itemData(languageIndex).toString() != Settings::getInstance()->getLanguage() );
}

void SettingsForm::on_lineEditBackupPrefix_textEdited( QString backupPrefix )
{
	formChanged = ( backupPrefix != Settings::getInstance()->getBackupPrefix() );
}

void SettingsForm::on_spinBoxNOfShownLastBackups_valueChanged( int i )
{
	formChanged = (i != Settings::getInstance()->getNOfLastBackups());
}

void SettingsForm::on_btnDefaultPrefix_clicked()
{
	this->lineEditBackupPrefix->setText( Settings::getInstance()->getLocalHostName() );
	formChanged = true;
}

void SettingsForm::on_checkBoxShowHiddenFiles_stateChanged( int state ) {
	formChanged = ((state==Qt::Checked) != Settings::getInstance()->getShowHiddenFilesAndFolders());
}

void SettingsForm::on_checkBoxKeepDeletedFiles_stateChanged( int state ) {
	formChanged = ((state==Qt::Checked) != (!Settings::getInstance()->getDeleteExtraneousItems()));
}
