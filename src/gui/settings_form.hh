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

#ifndef SETTINGS_FORM_HH
#define SETTINGS_FORM_HH

#include <QWidget>
#include <QMessageBox>
#include <QSettings>

#include "ui_settings_form.h"

#include "model/main_model.hh"

/**
 * The SettingsForm class is the form displaying the settings
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class SettingsForm : public QWidget, private Ui::SettingsForm
{
	Q_OBJECT
public:
	/**
	 * Constructs a SettingsForm which is a child of parent with a given model
	 */
	SettingsForm ( QWidget *parent = 0, MainModel *model = 0 );

	/**
	 * Destroys the SettingsForm
	 */
	virtual ~SettingsForm();
	
	/**
	 * Reloads the settings values
	 */
	void reload();
	bool onLeave();
	
private:
	MainModel *model;
	bool formChanged;
	
signals:
	void updateOverviewFormLastBackupsInfo();

private slots:
	void save();
	void reset();
	void on_btnDefaultServer_pressed();
	void on_btnDefaultServerKey_pressed();
	void on_btnDefaultPrefix_pressed();
	void on_lineEditUsername_textEdited ( QString username );
	void on_lineEditServer_textEdited ( QString serverName );
	void on_lineEditServerKey_textEdited ( QString serverKey );
	void on_lineEditBackupPrefix_textEdited( QString backupPrefix );
	void on_comboBoxLanguage_currentIndexChanged ( int languageIndex );
	void on_spinBoxNOfShownLastBackups_valueChanged( int i );
};

#endif
