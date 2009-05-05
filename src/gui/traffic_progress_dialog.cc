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
#include <QFileInfo>

#include "gui/traffic_progress_dialog.hh"
#include "settings/settings.hh"
#include "utils/debug_timer.hh"
#include "utils/string_utils.hh"


TrafficProgressDialog::TrafficProgressDialog( const QString& title )
{
	setupUi ( this );
	setWindowTitle( title );
	setAttribute( Qt::WA_DeleteOnClose );

	this->labelError->setVisible( false );
	this->textEditError->setVisible( false );
	this->isErrorVisible = false;
	this->lastUpdate = QTime::currentTime();
}

TrafficProgressDialog::~TrafficProgressDialog()
{
}

void TrafficProgressDialog::appendInfo( const QString& info )
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

void TrafficProgressDialog::flushCache()
{
	this->textEditOutput->append( this->outputCache  );
	this->outputCache.clear();
}

void TrafficProgressDialog::appendError( const QString& error )
{
	if ( !this->isErrorVisible )
	{
		this->labelError->setVisible( true );
		this->textEditError->setVisible( true );
		this->isErrorVisible = true;
	}
	this->textEditError->append( error  );
}

void TrafficProgressDialog::on_btnCancel_pressed()
{
	if (emit abort()) {
		qDebug() << "BackupProgress::on_btnCancel_pressed(): switching Button-visibility";
		this->btnClose->setEnabled(true);
		this->btnCancel->setEnabled(false);
	}
}

void TrafficProgressDialog::finished()
{
	flushCache();
	this->btnClose->setEnabled( true );
	this->btnCancel->setEnabled( false );
}

void TrafficProgressDialog::closeEvent( QCloseEvent * event )
{
	if (!this->btnClose->isEnabled()) event->ignore();
}

void TrafficProgressDialog::displayInfoFilename(const QString& filename)
{
	int curWidth = this->label_currentFile->fontMetrics().width( filename );
	int maxWidth = (int)(this->width() - label_currentFile->x()*0.9f);
	int maxChar_approx = (int)(maxWidth * filename.length() / curWidth);
	this->label_currentFile->setText(StringUtils::filenameShrink(filename, maxChar_approx));
	this->progressBar->setValue((this->progressBar->value()+1) % this->progressBar->maximum()); // TODO delete when progressBar is activated
}

