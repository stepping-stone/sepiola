/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2017 stepping stone GmbH
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

#include <QLabel>

#include "gui/about_dialog.hh"
#include "settings/settings.hh"

const int AboutDialog::MAX_IMAGE_HEIGHT = 75; 
const int AboutDialog::MAX_IMAGE_WIDTH = 150; 

AboutDialog::AboutDialog()
{
    setupUi(this);
    this->setBackgroundRole(QPalette::Base);
    Settings* settings = Settings::getInstance();
    this->labelVersion->setText(tr( "%1\nVersion %2" ).arg(settings->getApplicationName(), Settings::VERSION));

#ifdef IS_RESELLER
    this->labelSupport->setText(settings->getResellerAddress());

    // rescale image if needed
    if ((this->imageAbout->pixmap()->height() > AboutDialog::MAX_IMAGE_HEIGHT)
	|| (this->imageAbout->pixmap()->width() > AboutDialog::MAX_IMAGE_WIDTH))
    {
	this->imageAbout->setPixmap(
		this->imageAbout->pixmap()->scaled(
		    AboutDialog::MAX_IMAGE_WIDTH,
		    AboutDialog::MAX_IMAGE_HEIGHT,
		    Qt::KeepAspectRatio, Qt::SmoothTransformation)
		);
    }
#endif
}
