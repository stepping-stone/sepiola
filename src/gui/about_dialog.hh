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

#ifndef ABOUT_DIALOG_HH
#define ABOUT_DIALOG_HH

#include "ui_about_dialog.h"

/**
 * The AboutDialog class provides a dialog showing information about the owner of the application
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class AboutDialog : public QDialog, private Ui::AboutDialog
{
	Q_OBJECT
	static const int MAX_IMAGE_HEIGHT;
	static const int MAX_IMAGE_WIDTH;
	
public:

	/**
	 * Constructs an AboutDialog
	 */
	AboutDialog();
	
	/**
	 * Destroys the Aboutdialog
	 */
	virtual ~AboutDialog();
};

#endif
