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

#ifndef PASSWORD_DIALOG_HH
#define PASSWORD_DIALOG_HH

#include <QDialog>

#include "ui_password_dialog.h"

/**
 * The PasswordDialog class provides a dialog asking for username and password
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class PasswordDialog : public QDialog, private Ui::PasswordDialog
{
	Q_OBJECT
	
public:
	/**
	 * Constructs a PasswordDialog
	 * @param username name of the user
	 * @param isUsernameEditable indicates if the user is allowed to change the username
	 */
	PasswordDialog( const QString& username, const bool& isUsernameEditable );
	
	/**
	 * Destroys the PasswordDialog
	 */
	virtual ~PasswordDialog();
	void setDialogMessage(const QString& msg);

signals:
	void abort();
	void processPasswordReturnValues(const QString& username, const QString& password, const bool isUsernameEditable);
	
private slots:
	void on_btnOk_clicked();
	void on_btnCancel_clicked();
	void reject();

private:
	bool isUsernameEditable;
};

#endif
