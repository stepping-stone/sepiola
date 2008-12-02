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

#include "gui/settings_form.hh"

SettingsForm::SettingsForm ( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );
	this->model = model;
	// change the label of ok and cancel button to save and cancel
	QList<QAbstractButton*> buttons = this->buttonBox->buttons();
	foreach( QAbstractButton* button, buttons )
	{
		switch( this->buttonBox->buttonRole( button ) )
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
	reload();
}


SettingsForm::~SettingsForm()
{}

void SettingsForm::reload()
{
	Settings* settings = Settings::getInstance();
	this->lineEditUsername->setText( settings->getServerUserName() );
	this->lineEditServer->setText( settings->getServerName() );
	this->lineEditServerKey->setText( settings->getServerKey() );
	this->lineEditBackupPrefix->setText( settings->getBackupPrefix() );

	this->comboBoxLanguage->clear();
	foreach( QString language, settings->getSupportedLanguages() )
	{
		this->comboBoxLanguage->addItem( language );
	}
	this->comboBoxLanguage->setCurrentIndex( settings->getLanguageIndex() );
	QObject::connect( this->buttonBox, SIGNAL( accepted() ),
						this, SLOT( save() ) );
	QObject::connect( this->buttonBox, SIGNAL( rejected() ),
						this, SLOT( reset() ) );
	formChanged = false;
}

void SettingsForm::save()
{
	if ( formChanged )
	{
		Settings* settings = Settings::getInstance();
		settings->saveServerUserName( this->lineEditUsername->text() );
		settings->saveServerName( this->lineEditServer->text() );
		settings->saveServerKey( this->lineEditServerKey->text() );
		settings->saveBackupPrefix( this->lineEditBackupPrefix->text() );
		settings->saveLanguageIndex( this->comboBoxLanguage->currentIndex() );

		QMessageBox::information( this, tr( "Settings saved" ),
										tr( "Settings have been saved." ) );
		formChanged = false;
	}
}

void SettingsForm::reset()
{
	if ( formChanged )
	{
		int answer = QMessageBox::warning ( this, tr ( "Unsaved settings" ),
																		 tr ( "Your settings have been modified.\n"
																					"Do you want to restore the original settings?" ),
																		 QMessageBox::Yes | QMessageBox::No );
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

void SettingsForm::on_lineEditUsername_textEdited ( QString username )
{
	formChanged = true;
}

void SettingsForm::on_lineEditServer_textEdited ( QString serverName )
{
	formChanged = true;
}

void SettingsForm::on_lineEditServerKey_textEdited ( QString serverKey )
{
	formChanged = true;
}

void SettingsForm::on_comboBoxLanguage_currentIndexChanged ( int languageIndex )
{
	formChanged = true;
}

void SettingsForm::on_lineEditBackupPrefix_textEdited( QString backupPrefix )
{
	formChanged = true;
}

void SettingsForm::on_btnDefaultServer_pressed()
{
	this->lineEditServer->setText( Settings::getInstance()->getDefaultServerName() );
	formChanged = true;
}

void SettingsForm::on_btnDefaultServerKey_pressed()
{
	this->lineEditServerKey->setText( Settings::getInstance()->getDefaultServerKey() );
	formChanged = true;
}

void SettingsForm::on_btnDefaultPrefix_pressed()
{
	this->lineEditBackupPrefix->setText( Settings::getInstance()->getLocalHostName() );
	formChanged = true;
}
