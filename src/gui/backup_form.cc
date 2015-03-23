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

#include <QDebug>
#include <QMessageBox>
#include <QShortcut>

#include "settings/settings.hh"
#include "gui/backup_form.hh"
#include "gui/pattern_dialog.hh"
#include "model/scheduled_task.hh"
#include "model/main_model.hh"
#include "model/local_dir_model.hh"
#include "utils/log_file_utils.hh"

// default const values
/* it's not worth at this moment to add a new settings-variable, therefore default-schedule-values are set here */
const QTime BackupForm::default_schedule_time = QTime(12,0,0,0);
const int BackupForm::default_schedule_minutesAfterStartup = 10;


BackupForm::BackupForm( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );
	this->model = model;
	this->localDirModel = this->model->getLocalDirModel();

	this->reload();

	QObject::connect( this, SIGNAL( updateOverviewFormScheduleInfo() ),
					  parent, SIGNAL ( updateOverviewFormScheduleInfo() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormLastBackupsInfo() ),
					  parent, SIGNAL ( updateOverviewFormLastBackupsInfo() ) );
	QObject::connect( this->buttonBox, SIGNAL( accepted() ), this, SLOT( save() ) );
	QObject::connect( this->buttonBox, SIGNAL( rejected() ), this, SLOT( reset() ) );
}

BackupForm::~BackupForm()
{
	QObject::disconnect( this->buttonBox, SIGNAL( accepted() ), this, SLOT( save() ) );
	QObject::disconnect( this->buttonBox, SIGNAL( rejected() ), this, SLOT( reset() ) );
}

void BackupForm::reload() {
	Settings* settings = Settings::getInstance();
	this->localDirModel->setSelectionRules(	settings->getLastBackupSelectionRules() );
	this->treeView->setModel( localDirModel );
	this->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	this->treeView->setColumnWidth( 0, 280 );
	this->expandSelectedBranches();


		// disable option "minutes after booting" if the scheduler does not support it
	if ( !this->model->isSchedulingOnStartSupported() )
	{
		this->radioButtonMinutesAfterBooting->setEnabled( false );
		this->spinBoxMinutesAfterBooting->setEnabled( false );
	}

	ScheduledTask rule = settings->getScheduleRule();
	switch (rule.getType()) {
		case ScheduleRule::NEVER:
			this->radioButtonNoSchedule->setChecked(true);
			on_radioButtonNoSchedule_clicked();
		break;
		case ScheduleRule::AFTER_BOOT:
			this->radioButtonMinutesAfterBooting->setChecked(true);
			on_radioButtonMinutesAfterBooting_clicked();
			this->spinBoxMinutesAfterBooting->setValue(rule.getMinutesAfterStartup());
		break;
		case ScheduleRule::AT_WEEKDAYS_AND_TIME:
			this->radioButtonDaily->setChecked(true);
			on_radioButtonDaily_clicked();
			this->checkBoxMonday->setChecked(rule.getWeekdays().contains(ScheduleRule::MONDAY));
			this->checkBoxTuesday->setChecked(rule.getWeekdays().contains(ScheduleRule::TUESDAY));
			this->checkBoxWednesday->setChecked(rule.getWeekdays().contains(ScheduleRule::WEDNESDAY));
			this->checkBoxThursday->setChecked(rule.getWeekdays().contains(ScheduleRule::THURSDAY));
			this->checkBoxFriday->setChecked(rule.getWeekdays().contains(ScheduleRule::FRIDAY));
			this->checkBoxSaturday->setChecked(rule.getWeekdays().contains(ScheduleRule::SATURDAY));
			this->checkBoxSunday->setChecked(rule.getWeekdays().contains(ScheduleRule::SUNDAY));
			this->timeEditTime->setTime(rule.getTimeToRun());
		break;
	}
}

void BackupForm::expandSelectedBranches()
{
	foreach ( QString location, this->localDirModel->getSelectionRules().keys() ) {
		QString loc = StringUtils::parentDir(location);
		QModelIndex index = localDirModel->index( loc );
		while (index.isValid()) {
			this->treeView->expand( index ); // if possible
			index = index.parent();
		}
	}
}

void BackupForm::save()
{
	qDebug() << "BackupForm::save()";
	if ( this->localDirModel->getSelectionRules().size() == 0 && !this->radioButtonNoSchedule->isChecked() )
	{
		QMessageBox::information( this, tr( "Empty backup list" ), tr( "No items have been selected for scheduling" ) );
		return;
	}
	this->schedule();
	// Settings::getInstance()->saveBackupSelectionRules( this->model->getLocalDirModel()->getSelectionRules() ); // this is not necessary, because it is called in method schedule()
}

void BackupForm::reset()
{
	qDebug() << "BackupForm::reset()";
	int answer = QMessageBox::warning( this, tr( "Reset" ), tr( "Are you sure you want to reset all form values?" ), QMessageBox::Yes | QMessageBox::Cancel );

	if (answer == QMessageBox::Yes)
		this->reload();
}

void BackupForm::schedule()
{
	ScheduledTask scheduleTask;
	if ( this->radioButtonDaily->isChecked() )
	{
		QTime time = this->timeEditTime->time();
		QSet<ScheduleRule::Weekdays> wd;
		if (this->checkBoxMonday->checkState() == Qt::Checked) wd.insert(ScheduleRule::MONDAY);
		if (this->checkBoxTuesday->checkState() == Qt::Checked) wd.insert(ScheduleRule::TUESDAY);
		if (this->checkBoxWednesday->checkState() == Qt::Checked) wd.insert(ScheduleRule::WEDNESDAY);
		if (this->checkBoxThursday->checkState() == Qt::Checked) wd.insert(ScheduleRule::THURSDAY);
		if (this->checkBoxFriday->checkState() == Qt::Checked) wd.insert(ScheduleRule::FRIDAY);
		if (this->checkBoxSaturday->checkState() == Qt::Checked) wd.insert(ScheduleRule::SATURDAY);
		if (this->checkBoxSunday->checkState() == Qt::Checked) wd.insert(ScheduleRule::SUNDAY);

		bool validSelection = wd.size() > 0;;

		if ( !validSelection )
		{
			this->model->infoDialog( tr( "Check at least one day" ) );
			return;
		}
		scheduleTask = ScheduledTask(wd, time);
	}
	else if ( this->radioButtonMinutesAfterBooting->isChecked() )
	{
		int delay = this->spinBoxMinutesAfterBooting->value();
		scheduleTask = ScheduledTask(delay);
	}
	else if ( this->radioButtonNoSchedule->isChecked() )
	{
		scheduleTask = ScheduledTask();
	}
	this->model->schedule( this->model->getLocalDirModel()->getSelectionRules(), scheduleTask );
	qDebug() << "BackupForm::schedule(): emit updateOverviewFormScheduleInfo()";
	emit updateOverviewFormScheduleInfo();
}


void BackupForm::runBackupNow()
{
	if ( this->localDirModel->getSelectionRules().size() == 0 )
	{
		QMessageBox::information( this, tr( "Empty backup list" ), tr( "No items have been selected for backup" ));
		return;
	}
	this->model->showProgressDialogSlot( tr( "Backup" ) );
	qDebug() << "BackupForm::on_btnBackup_clicked()" << this->model->getLocalDirModel()->getSelectionRules();
	this->model->backup( this->model->getLocalDirModel()->getSelectionRules(), false );
	//this->model->backup( backupItems, includePatternList, excludePatternList, deleteExtraneous, false );
	emit updateOverviewFormLastBackupsInfo();
}

QString BackupForm::patternListToString( QStringList patternList )
{
	QString result;
	for ( int i=0; i<patternList.size(); i++ )
	{
		result.append( patternList.at( i ) );
		if ( i < patternList.size() -1 )
		{
			result.append( ", ");
		}
	}
	return result;
}

void BackupForm::disableScheduleOptions() {
	this->spinBoxMinutesAfterBooting->setEnabled(false);
	this->spinBoxMinutesAfterBooting->setValue( default_schedule_minutesAfterStartup );
	this->timeEditTime->setEnabled(false);
	this->timeEditTime->setTime( default_schedule_time );
	QList<QCheckBox*> weekdaysCheckboxes;
	weekdaysCheckboxes << this->checkBoxMonday << this->checkBoxTuesday << this->checkBoxWednesday << this->checkBoxThursday << this->checkBoxFriday << this->checkBoxSaturday << this->checkBoxSunday;
	foreach (QCheckBox* cb, weekdaysCheckboxes ) {
		cb->setChecked(true);
		cb->setEnabled(false);
	}
}

void BackupForm::on_radioButtonNoSchedule_clicked() {
	disableScheduleOptions();
}

void BackupForm::on_radioButtonMinutesAfterBooting_clicked() {
	disableScheduleOptions();
	this->spinBoxMinutesAfterBooting->setEnabled(true);
}

void BackupForm::on_radioButtonDaily_clicked() {
	disableScheduleOptions();
	this->timeEditTime->setEnabled(true);
	QList<QCheckBox*> weekdaysCheckboxes;
	weekdaysCheckboxes << this->checkBoxMonday << this->checkBoxTuesday << this->checkBoxWednesday << this->checkBoxThursday << this->checkBoxFriday << this->checkBoxSaturday << this->checkBoxSunday;
	foreach (QCheckBox* cb, weekdaysCheckboxes ) {
		cb->setEnabled(true);
	}
}


QStringList BackupForm::getSelectedFilesAndDirs()
{
	QModelIndexList indexes = this->treeView->selectionModel()->selectedIndexes();
	QStringList items;
	// save current value of localDirModel->resolveSymlinks to reset after getting filePaths of the items
	bool bkup_ResolveSymbolicLinks = localDirModel->resolveSymlinks();
	localDirModel->setResolveSymlinks(false);
	foreach ( QModelIndex index, indexes )
	{
		if ( index.column() == 0 )
		{
			items << localDirModel->filePath( index );
		}
	}
	localDirModel->setResolveSymlinks(bkup_ResolveSymbolicLinks);
	return items;
}

void BackupForm::showHiddenFilesAndFolders(bool show)
{
	qDebug() << "BackupForm::showHiddenFilesAndFolders()";

	if (show)
		this->localDirModel->setFilter(this->localDirModel->filter() | QDir::Hidden);
	else
		this->localDirModel->setFilter(this->localDirModel->filter() & (~QDir::Hidden));
}
