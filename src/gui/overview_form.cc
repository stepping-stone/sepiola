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
#include <QPicture>
#include <QLabel>
#include <QMessageBox>
#include <QImageReader>
#include <math.h>

#include "gui/overview_form.hh"
#include "settings/settings.hh"
#include "utils/debug_timer.hh"
#include "model/scheduled_task.hh"
#include "model/backup_task.hh"

OverviewForm::OverviewForm( QWidget *parent, MainModel *model ) : QWidget( parent )
{
	COLOR_BLACK = qRgba( 0, 0, 0, 255 );
	COLOR_WHITE = qRgba( 255, 255, 255, 255 );
	COLOR_BACKUP = qRgba( 255, 0, 0, 255 );
	COLOR_SNAPSHOT = qRgba( 255, 175, 0, 255 );
	COLOR_FREE = qRgba( 0, 191, 0, 255 );

	setupUi( this );
	initialized = false;

	this->model = model;

	// adjust content of change-quota-label
	Settings* settings = Settings::getInstance();
	QString uid_param = settings->getQuotaModificationUrlUidParam();
	uid_param = (uid_param == "") ? "$UID$" : uid_param;
	if (settings->getQuotaModificationUrl() != "") {
		this->labelInfoChangeQuota->setText(QObject::tr("<a href=\"%1\">Change quota</a>").arg(settings->getQuotaModificationUrl().replace(uid_param, settings->getServerUserName())));
		this->labelInfoChangeQuota->setOpenExternalLinks(true);
	} else {
		this->labelInfoChangeQuota->setText("");
		this->labelInfoChangeQuota->setVisible(false);
	}

	// refresh form
	this->refreshSpaceStatistic();
	this->refreshLastBackupsOverview();
	this->refreshScheduleOverview();

}

OverviewForm::~OverviewForm()
{
}

void OverviewForm::on_btnBackupNow_clicked()
{
	QMessageBox::information( this, tr( "Backup" ), tr( "Not implemented yet, unclear which items to backup" ));
}

void OverviewForm::refreshLastBackupsOverview()
{
	qDebug() << "OverviewForm::refreshLastBackupsOverview()";
	Settings* settings = Settings::getInstance();
	QStringList fieldnames_status;
	fieldnames_status << "labelIconStatusLastBackup_%1" << "labelStatusLastBackup_%1" << "labelDateLastBackup_%1";
	QImage img;

	// fill map with a pixmap for each status
	QMap<ConstUtils::StatusEnum, QPixmap> status_pixmap;
	img.load( ":/main/sign_ok.svg" );
	status_pixmap.insert( ConstUtils::STATUS_OK, QPixmap::fromImage( img ) );
	img.load( ":/main/sign_error.svg" );
	status_pixmap.insert( ConstUtils::STATUS_ERROR, QPixmap::fromImage( img ) );
	img.load( ":/main/sign_warning.svg" );
	status_pixmap.insert( ConstUtils::STATUS_WARNING, QPixmap::fromImage( img ) );
	QList<BackupTask> lastBackups = settings->getLastBackups();
	int n = std::min( settings->getNOfLastBackups(), lastBackups.size() );
	// fill status-content of last backups into form
	for ( int i = 0; i < MAX_LAST_BACKUP_INFOS; i++ )
	{
		bool isHidden = (i >= n);
		QLabel* lab_icon = this->findChild<QLabel *>( fieldnames_status[0].arg( i ) );
		lab_icon->setHidden( isHidden );
		QLabel* lab_status = this->findChild<QLabel *>( fieldnames_status[1].arg( i ) );
		lab_status->setHidden( isHidden );
		QLabel* lab_date = this->findChild<QLabel *>( fieldnames_status[2].arg( i ) );
		lab_date->setHidden( isHidden );
		if ( !isHidden )
		{
			BackupTask lastBackup = lastBackups.at( i );
			ConstUtils::StatusEnum status = lastBackup.getStatus();
			lab_icon->setPixmap( status_pixmap.value( status ) );
			lab_status->setText( lastBackup.getStatusText() );
			lab_date->setText( lastBackup.getDateTime().toString() );
		}
	}
}

void OverviewForm::refreshScheduleOverview()
{
	// fills information on next scheduled backup into form
	qDebug() << "OverviewForm::refreshScheduleOverview()";
	ScheduledTask myTask = Settings::getInstance()->getScheduleRule();
	if ( myTask.getType() != ScheduleRule::NEVER )
	{
		this->labelNextBackup->setText( QObject::tr( "Scheduled" ) );
		this->labelDateNextBackup->setText( myTask.toString() );
	}
	else
	{
		this->labelNextBackup->setText( QObject::tr( "Not scheduled" ) );
		this->labelDateNextBackup->setText( "" );
	}
}


/**
* refreshes the part of the form, where the statistic of quota and used space is shown
*/
void OverviewForm::refreshSpaceStatistic()
{
	qDebug() << "OverviewForm::refreshSpaceStatistic()";
	QList<int> quotaValues = this->model->getServerQuota();
	// definition of formats for output
	QString format_percent = QString( "%1\%" ), format_MB = QString( "%1 MB" ), format_GB = QString( "%1 GB" ), format_TB = QString( "%1 TB" );
	QStringList sizeNames; sizeNames << "Backup" << "Snapshot" << "Free" << "Quota";
	int precision_MB = 0, precision_GB = 1, precision_TB = 2;

	int quota, backup, snapshot;
	qDebug() << "OverviewForm::refreshSpaceStatistic(): quotaValues.size()" << quotaValues.size();
	if (quotaValues.size() == 3) {
		quota = quotaValues.at(0);
		backup = quotaValues.at(1);
		snapshot = quotaValues.at(2);
		this->labelSpaceAvailable->setPixmap( this->getSpaceVisualization( quota, backup, snapshot ) );

		// The following 3 lines could also be in the constructor
		this->labelLegendBackup->setPixmap( this->getSpaceVisualization( 100, 100, 0, 16, 1 ) );
		this->labelLegendSnapshot->setPixmap( this->getSpaceVisualization( 100, 0, 100, 16, 1 ) );
		this->labelLegendFree->setPixmap( this->getSpaceVisualization( 100, 0, 0, 16, 1 ) );

		QList<float> sizes; sizes << backup << snapshot << ( quota - backup - snapshot ) << quota;

		// set field-text of all space-fields
		for ( int i = 0; i < sizeNames.size(); i++ )
		{
			QLabel * el_abs = this->findChild<QLabel *>( QString( "labelSpace" ) + sizeNames[i] );
			QLabel * el_percent = this->findChild<QLabel *>( QString( "labelSpacePercent" ) + sizeNames[i] );
			if ( el_abs != 0 )
			{
				if ( sizes[i] >= 1024 * 1024 )
					el_abs->setText( format_TB.arg( sizes[i] / 1024 / 1024, 0, 'f', precision_TB ) );
				else if ( sizes[i] >= 1024 )
					el_abs->setText( format_GB.arg( sizes[i] / 1024, 0, 'f', precision_GB ) );
				else
					el_abs->setText( format_MB.arg( sizes[i], 0, 'f', precision_MB ) );
			}
			if ( el_percent != 0 )
			{
				el_percent->setText( format_percent.arg( 100.0f*sizes[i] / sizes[sizes.size()-1], 0, 'f', 1 ) );
			}
		}
	} else {
		QPixmap emptyPixmap;
		this->labelSpaceAvailable->setPixmap( emptyPixmap );
		this->labelLegendBackup->setPixmap( emptyPixmap );
		this->labelLegendSnapshot->setPixmap( emptyPixmap );
		this->labelLegendFree->setPixmap( emptyPixmap );

		// set field-text of all space-fields
		for ( int i = 0; i < sizeNames.size(); i++ )
		{
			QLabel * el_abs = this->findChild<QLabel *>( QString( "labelSpace" ) + sizeNames[i] );
			QLabel * el_percent = this->findChild<QLabel *>( QString( "labelSpacePercent" ) + sizeNames[i] );
			if ( el_abs != 0 )
			{
				el_abs->setText( QObject::tr("N/A") );
			}
			if ( el_percent != 0 )
			{
				el_percent->setText( QObject::tr("N/A") );
			}
		}
	}
}

/**
* returns a QPixmap-bar, showing used, snapshot and free space in different colors
*/
QPixmap OverviewForm::getSpaceVisualization( int quota, int used, int snapshot, int imgH, int imgW )
{
	QImage spaceImg = QImage( imgW, imgH, QImage::Format_RGB32 );
	spaceImg.fill( COLOR_WHITE );
	float thresholdX = 0.5f; // brightness at which original color is left untouched, below darkend, above brightend
	int i = 0;
	for ( int j = 0; j < imgH; j++ )
	{
		QRgb* rgbData = ( QRgb* )( spaceImg.scanLine( j ) );
		float b = ( float )pow( sin(( 75 + ( j / ( imgH - 1.0 ) ) * 110.0 ) / 180.0 * 3.141592 ), 2.0 ) * 0.7f + 0.2;
		QRgb c = linIP( COLOR_BLACK, COLOR_BACKUP, COLOR_WHITE, thresholdX, b );
		for ( i = 0; i < floor(( float )imgW*used / quota ); i++ )
			rgbData[i] = c;
		c = linIP( COLOR_BLACK, COLOR_SNAPSHOT, COLOR_WHITE, thresholdX, b );
		for ( ;i < floor(( float )imgW*( used + snapshot ) / quota );i++ )
			rgbData[i] = c;
		c = linIP( COLOR_BLACK, COLOR_FREE, COLOR_WHITE, thresholdX, b );
		for ( ;i < imgW;i++ )
			rgbData[i] = c;
	}
	return QPixmap::fromImage( spaceImg );
}

QRgb OverviewForm::linIP( QRgb v0, QRgb vMid, QRgb v1, float mid, float x )
{
	float x0, x1;
	QColor y0, y1;
	if ( x <= mid )
	{
		x0 = 0; x1 = mid; y0 = QColor( v0 ); y1 = QColor( vMid );
	}
	else
	{
		x0 = mid; x1 = 1; y0 = QColor( vMid ); y1 = QColor( v1 );
	}
	float q0 = ( x - x0 ) / ( x1 - x0 );
	float q1 = ( x1 - x ) / ( x1 - x0 );
	return qRgba(( int )( q0*y1.red() + q1*y0.red() ), ( int )( q0*y1.green() + q1*y0.green() ), ( int )( q0*y1.blue() + q1*y0.blue() ), ( int )( q0*y1.alpha() + q1*y0.alpha() ) );
}

