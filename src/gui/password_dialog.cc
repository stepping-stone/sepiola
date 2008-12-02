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

#include "settings/settings.hh"
#include "gui/password_dialog.hh"
#include "utils/log_file_utils.hh"

PasswordDialog::PasswordDialog( const QString& username, const bool& isUsernameEditable )
{
	setupUi ( this );
	
	this->lineEditUsername->setText( username );
	this->isUsernameEditable = isUsernameEditable;
	if ( !isUsernameEditable )
	{
		this->lineEditUsername->setEnabled( false );
	}
}

PasswordDialog::~PasswordDialog()
{
}

void PasswordDialog::setDialogMessage(const QString& msg)
{
	this->labelMessage->setText(msg);
}

void PasswordDialog::on_btnOk_pressed()
{
	emit processPasswordReturnValues(this->lineEditUsername->text(), this->lineEditPassword->text(), isUsernameEditable);
	this->done(Accepted);
}

void PasswordDialog::on_btnCancel_pressed()
{
	this->done(Rejected);
	emit abort();
}

void PasswordDialog::reject()
{
}
