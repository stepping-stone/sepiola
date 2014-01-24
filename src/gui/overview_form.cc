/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2011 stepping stone GmbH
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
#include <cmath>

#include "gui/overview_form.hh"
#include "settings/settings.hh"
#include "utils/debug_timer.hh"
#include "model/scheduled_task.hh"
#include "model/backup_task.hh"
#include "model/main_model.hh"
#include "model/scheduled_task.hh"
#include "model/space_usage_model.hh"

OverviewForm::OverviewForm( QWidget *parent, MainModel *model ) :
    QWidget( parent ),
    spaceUsageModel(new SpaceUsageModel(this))
{
	setupUi( this );

	this->model = model;
    this->stackedBarView->setModel(spaceUsageModel);

	// adjust content of change-quota-label
	Settings* settings = Settings::getInstance();
	QString uid_param = settings->getQuotaModificationUrlUidParam();
	uid_param = (uid_param == "") ? "$UID$" : uid_param;
	if (settings->getQuotaModificationUrl() != "") {
		this->labelInfoChangeQuota->setText(QObject::tr("<a href=\"%1\">Change quota</a> (opens a browser window)").arg(settings->getQuotaModificationUrl().replace(uid_param, settings->getServerUserName())));
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
    delete spaceUsageModel;
}

void OverviewForm::on_btnBackupNow_clicked()
{
	emit startBackupNow();
}

void OverviewForm::refreshLastBackupsOverview()
{
	qDebug() << "OverviewForm::refreshLastBackupsOverview()";
	Settings* settings = Settings::getInstance();
	QStringList fieldnames_status;
	fieldnames_status << "labelIconStatusLastBackup_%1" << "labelStatusLastBackup_%1" << "labelDayLastBackup_%1" << "labelDateLastBackup_%1" << "labelTimeLastBackup_%1";
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
		QLabel* lab_day = this->findChild<QLabel *>( fieldnames_status[2].arg( i ) );
		lab_day->setHidden( isHidden );
		QLabel* lab_date = this->findChild<QLabel *>( fieldnames_status[3].arg( i ) );
		lab_date->setHidden( isHidden );
		QLabel* lab_time = this->findChild<QLabel *>( fieldnames_status[4].arg( i ) );
		lab_time->setHidden( isHidden );

		if ( !isHidden )
		{
			BackupTask lastBackup = lastBackups.at( i );
			ConstUtils::StatusEnum status = lastBackup.getStatus();
			lab_icon->setPixmap( status_pixmap.value( status ) );
			lab_status->setText( lastBackup.getStatusText() );
			lab_day->setText( lastBackup.getDateTime().toString("dddd") );
			lab_date->setText( lastBackup.getDateTime().toString("dd.MM.yyyy") );
			lab_time->setText( lastBackup.getDateTime().toString("hh:mm") );
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

		// Get the next execution string which will be of format:
		// weekday,\tdd.MM.yyyy   hh:mm
		QString next_task = myTask.toString();

		// Prepare the regex to extract weekday, date and time from the task
		// string
		QString next_weekday;
        QString next_date;
        QString next_time;
        QRegExp regex("(\\w*),\t(\\d{2}.\\d{2}.\\d{4})\\s*(\\d{2}:\\d{2})");

        // Parse the next_task in order to extract weekday, date and time and
        int pos = regex.indexIn( next_task );
        if (pos > -1) {
            next_weekday = regex.cap(1);
            next_date = regex.cap(2);
            next_time = regex.cap(3);
        } else
        {
            next_date = "";
            next_time = "";
            next_weekday = "Not found";
        }

        // Set the corresponding label text
		this->labelDateNextBackup->setText( next_date );
		this->labelTimeNextBackup->setText( next_time );
		this->labelDayNextBackup->setText( next_weekday );

	}
	else
	{
		this->labelNextBackup->setText( QObject::tr( "Not scheduled" ) );
		this->labelDateNextBackup->setText( "" );
		this->labelDayNextBackup->setText( "" );
		this->labelTimeNextBackup->setText( "" );
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
	QString format_percent( "%1%" ), format_MB( "%1 MB" ), format_GB( "%1 GB" ), format_TB( "%1 TB" );
	QStringList sizeNames; sizeNames << "Backup" << "Snapshot" << "Free" << "Quota";
	int precision_MB = 0, precision_GB = 1, precision_TB = 2;

	int quota, backup, snapshot;
	qDebug() << "OverviewForm::refreshSpaceStatistic(): quotaValues.size()" << quotaValues.size();

	if (quotaValues.size() == 3) {
		quota = quotaValues.at(0);
		backup = quotaValues.at(1);
		snapshot = quotaValues.at(2);
        spaceUsageModel->setSpaceUsage(backup, snapshot, (quota - backup - snapshot), quota);

        this->labelLegendBackup->setPixmap(this->stackedBarView->legendIcon(spaceUsageModel->index(SpaceUsageModel::BACKUP, 0)));
        this->labelLegendSnapshot->setPixmap(this->stackedBarView->legendIcon(spaceUsageModel->index(SpaceUsageModel::INCREMENTAL, 0)));
        this->labelLegendFree->setPixmap(this->stackedBarView->legendIcon(spaceUsageModel->index(SpaceUsageModel::FREE, 0)));

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
        spaceUsageModel->setSpaceUsage(0, 0, 0, 0);

		// set field-text of all space-fields
		for ( int i = 0; i < sizeNames.size(); i++ )
		{
			QLabel * el_abs = this->findChild<QLabel *>( QString( "labelSpace" ) + sizeNames[i] );
			QLabel * el_percent = this->findChild<QLabel *>( QString( "labelSpacePercent" ) + sizeNames[i] );
			if ( el_abs != 0 )
			{
				el_abs->setTextFormat(Qt::RichText);
				el_abs->setText( QObject::tr("&mdash; GB") );
			}
			if ( el_percent != 0 )
			{
				el_percent->setTextFormat(Qt::RichText);
				el_percent->setText( QObject::tr("&mdash; %") );
			}
		}
	}
}
