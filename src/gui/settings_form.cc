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

#include "settings/settings.hh"
#include "gui/settings_form.hh"
#include "model/main_model.hh"

SettingsForm::SettingsForm( QWidget *parent, MainModel *model ) : QWidget( parent )
{
	setupUi( this );
	this->model = model;
	// Change the labels of accept and reject buttons to 'Save' and 'Discard'
	// and disable both until the form has been changed
	QList<QAbstractButton*> buttons = this->buttonBox->buttons();
	foreach( QAbstractButton* button, buttons )
	{
		switch ( this->buttonBox->buttonRole( button ) )
		{
			case QDialogButtonBox::AcceptRole:
				button->setText( tr( "Save" ) );
				button->setDisabled( true );
				break;
			case QDialogButtonBox::RejectRole:
				button->setText( tr( "Discard" ) );
				button->setDisabled( true );
				break;
			default:
				qWarning() << "Found unexpected button";
				break;
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

#ifdef Q_OS_WIN32
    checkBoxUseShadowCopy->setEnabled(true);
#else
    checkBoxUseShadowCopy->setChecked(false);
    checkBoxUseShadowCopy->setCheckable(false);
    checkBoxUseShadowCopy->setEnabled(false);
#endif

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
	this->checkBoxVerboseLogging->setChecked( settings->getLogDebugMessages() );
	this->checkBoxUseShadowCopy->setChecked( settings->getDoSnapshot() );
	this->spinBoxBandwidthLimit->setValue( settings->getBandwidthLimit() );

	this->comboBoxLanguage->clear();
    for ( auto language: settings->getAvailableLanguages() )
		this->comboBoxLanguage->addItem( language.first, language.second );

	this->comboBoxLanguage->setCurrentIndex( this->comboBoxLanguage->findData(settings->getLanguage()) );
	onFormChange();
}

void SettingsForm::save()
{
	Settings* settings = Settings::getInstance();
	settings->saveServerUserName( this->lineEditUsername->text() );
	settings->saveBackupPrefix( this->lineEditBackupPrefix->text() );
	settings->saveLanguage( this->comboBoxLanguage->itemData(this->comboBoxLanguage->currentIndex()).toString() );
	settings->saveNOfLastBackups( this->spinBoxNOfShownLastBackups->value() );
	settings->saveShowHiddenFilesAndFolders( this->checkBoxShowHiddenFiles->isChecked() );
	settings->saveDeleteExtraneousItems( !this->checkBoxKeepDeletedFiles->isChecked() );
	settings->saveLogDebugMessages( this->checkBoxVerboseLogging->isChecked() );
        settings->saveDoSnapshot( this->checkBoxUseShadowCopy->isChecked() );
	settings->saveBandwidthLimit( this->spinBoxBandwidthLimit->value() );

	QMessageBox::information( this, tr( "Settings saved" ), tr( "Settings have been saved." ) );
	emit updateOverviewFormLastBackupsInfo();
	emit showHiddenFilesAndFolders( this->checkBoxShowHiddenFiles->isChecked() );
	onFormChange();
}

void SettingsForm::reset()
{
	qDebug() << "SettingsForm::reset()";
	int answer = QMessageBox::warning( this, tr( "Unsaved settings" ), tr( "Your settings have been modified.\n" "Do you want to restore your previous settings?" ), QMessageBox::Yes | QMessageBox::No );
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

void SettingsForm::onFormChange()
{
	Settings* settings = Settings::getInstance();
	if (
		this->lineEditUsername->text() != settings->getServerUserName() ||
		this->lineEditBackupPrefix->text() != settings->getBackupPrefix() ||
		this->spinBoxNOfShownLastBackups->value() != settings->getNOfLastBackups() ||
		this->checkBoxShowHiddenFiles->isChecked() != settings->getShowHiddenFilesAndFolders() ||
		this->checkBoxKeepDeletedFiles->isChecked() != !settings->getDeleteExtraneousItems() ||
		this->checkBoxVerboseLogging->isChecked() != settings->getLogDebugMessages() ||
#ifdef Q_OS_WIN32
		this->checkBoxUseShadowCopy->isChecked() != settings->getDoSnapshot() ||
#endif
		this->spinBoxBandwidthLimit->value() != settings->getBandwidthLimit() ||
		this->comboBoxLanguage->currentIndex() != this->comboBoxLanguage->findData(settings->getLanguage())
	)
	{
		formChanged = true;
	}
	else
	{
		formChanged = false;
	}

	QList<QAbstractButton*> buttons = this->buttonBox->buttons();
	foreach( QAbstractButton* button, buttons )
	{
		if ( this->buttonBox->buttonRole( button ) == QDialogButtonBox::AcceptRole || this->buttonBox->
buttonRole( button ) == QDialogButtonBox::RejectRole )
		{
			button->setDisabled( !formChanged );
		}
	}
}

void SettingsForm::on_lineEditUsername_textEdited()
{
	onFormChange();
}

void SettingsForm::on_comboBoxLanguage_currentIndexChanged()
{
	onFormChange();
}

void SettingsForm::on_lineEditBackupPrefix_textEdited()
{
	onFormChange();
}

void SettingsForm::on_spinBoxNOfShownLastBackups_valueChanged()
{
	onFormChange();
}

void SettingsForm::on_spinBoxBandwidthLimit_valueChanged()
{
	onFormChange();
}

void SettingsForm::on_btnDefaultPrefix_clicked()
{
	this->lineEditBackupPrefix->setText( Settings::getInstance()->getLocalHostName() );
	onFormChange();
}

void SettingsForm::on_checkBoxShowHiddenFiles_stateChanged()
{
	onFormChange();
}

void SettingsForm::on_checkBoxKeepDeletedFiles_stateChanged()
{
	onFormChange();
}

void SettingsForm::on_checkBoxVerboseLogging_stateChanged()
{
	onFormChange();
}

void SettingsForm::on_checkBoxUseShadowCopy_stateChanged()
{
	onFormChange();
}
