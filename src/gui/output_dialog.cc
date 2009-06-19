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

#include <QCloseEvent>
#include <QTimer>
#include <QDebug>

#include "gui/output_dialog.hh"
#include "settings/settings.hh"
#include "utils/debug_timer.hh"

OutputDialog::OutputDialog( const QString& title )
{
	setupUi ( this );
	setWindowTitle( title );
	setAttribute( Qt::WA_DeleteOnClose );

	this->labelError->setVisible( false );
	this->textEditError->setVisible( false );
	this->isErrorVisible = false;
	this->lastUpdate = QTime::currentTime();
}

OutputDialog::~OutputDialog()
{
}

void OutputDialog::appendInfo( const QString& info )
{
	const int msecs_to_wait_for_flush = 500;
	QTime currentTime = QTime::currentTime();
	this->outputCache.append( info );
	if ( this->lastUpdate.addMSecs(msecs_to_wait_for_flush) <= currentTime )
	{
		// update, but only after a second, such that following messages ariving in the next second are also flushed
		this->lastUpdate = QTime::currentTime();
		QTimer::singleShot(msecs_to_wait_for_flush, this, SLOT(flushCache()));
	}
	else
	{
		// add a new line
		this->outputCache.append( Settings::getInstance()->getEOLCharacter() );
	}
}

void OutputDialog::flushCache()
{
	this->textEditOutput->append( this->outputCache  );
	this->outputCache.clear();
}

void OutputDialog::appendError( const QString& error )
{
	if ( !this->isErrorVisible )
	{
		this->labelError->setVisible( true );
		this->textEditError->setVisible( true );
		this->isErrorVisible = true;
	}
	this->textEditError->append( error  );
}

void OutputDialog::on_btnCancel_pressed()
{
	if (emit abort()) {
		qDebug() << "OutputDialog::on_btnCancel_pressed(): switching Button-visibility";
		this->btnClose->setEnabled(true);
		this->btnCancel->setEnabled(false);
	}
}

void OutputDialog::finished()
{
	flushCache();
	this->btnClose->setEnabled( true );
	this->btnCancel->setEnabled( false );
	emit refreshLastBackupOverview();
}

void OutputDialog::closeEvent( QCloseEvent * event )
{
	if (!this->btnClose->isEnabled()) event->ignore();
}

