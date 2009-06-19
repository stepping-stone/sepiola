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
#include <QMessageBox>
#include <QShortcut>

#include "gui/backup_form.hh"
#include "gui/pattern_dialog.hh"
#include "model/scheduled_task.hh"

#include "utils/log_file_utils.hh"

BackupForm::BackupForm( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );

	this->model = model;
	this->localDirModel = this->model->getLocalDirModel();
	this->treeView->setModel( localDirModel );
	this->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	this->treeView->setColumnWidth( 0, 300 );
	QModelIndex index = localDirModel->index( "/home/dsydler/" );
	do {
		this->treeView->expand( index );
		index = index.parent();
	} while (index.isValid());

	Settings* settings = Settings::getInstance();
	this->includePatternList = settings->getIncludePatternList();
	this->excludePatternList = settings->getExcludePatternList();
	this->lineEditIncludePattern->setText( patternListToString( includePatternList ) );
	this->lineEditExcludePattern->setText( patternListToString( excludePatternList ) );
	setDetailsVisible( false );

		// disable option "minutes after booting" if the scheduler does not support it
	if ( !this->model->isSchedulingOnStartSupported() )
	{
		this->radioButtonMinutesAfterBooting->setEnabled( false );
		this->spinBoxMinutesAfterBooting->setEnabled( false );
	}
	ScheduledTask rule = settings->getScheduleRule();
	switch (rule.getType()) {
		case ScheduleRule::NEVER: this->radioButtonNoSchedule->setChecked(true); break;
		case ScheduleRule::AFTER_BOOT: this->radioButtonMinutesAfterBooting->setChecked(true); break;
		case ScheduleRule::AT_WEEKDAYS_AND_TIME: this->radioButtonDaily->setChecked(true); break;
	}
	this->spinBoxMinutesAfterBooting->setValue(rule.getMinutesAfterStartup());
	this->checkBoxMonday->setChecked(rule.getWeekdays().contains(ScheduleRule::MONDAY));
	this->checkBoxTuesday->setChecked(rule.getWeekdays().contains(ScheduleRule::TUESDAY));
	this->checkBoxWednesday->setChecked(rule.getWeekdays().contains(ScheduleRule::WEDNESDAY));
	this->checkBoxThursday->setChecked(rule.getWeekdays().contains(ScheduleRule::THURSDAY));
	this->checkBoxFriday->setChecked(rule.getWeekdays().contains(ScheduleRule::FRIDAY));
	this->checkBoxSaturday->setChecked(rule.getWeekdays().contains(ScheduleRule::SATURDAY));
	this->checkBoxSunday->setChecked(rule.getWeekdays().contains(ScheduleRule::SUNDAY));
	this->timeEditTime->setTime(rule.getTimeToRun());
	
	new QShortcut( Qt::Key_F5, this, SLOT( refreshLocalDirModel() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormScheduleInfo() ),
					  parent, SIGNAL ( updateOverviewFormScheduleInfo() ) );
	QObject::connect( this, SIGNAL( updateOverviewFormLastBackupsInfo() ),
					  parent, SIGNAL ( updateOverviewFormLastBackupsInfo() ) );
}

BackupForm::~BackupForm()
{}

void BackupForm::on_btnSchedule_pressed()
{
	if ( this->localDirModel->getSelectionRules().size() == 0 && !this->radioButtonNoSchedule->isChecked() )
	{
		QMessageBox::information( this,
									tr( "Empty backup list" ),
									tr( "No items have been selected for scheduling" )
								);
		return;
	}
	/*ScheduleDialog scheduleDialog( this->model, backupItems, includePatternList, excludePatternList );
	scheduleDialog.exec();*/
	this->schedule();
}

void BackupForm::schedule()
{
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
		bool setDeleteFlag = this->checkBoxDeleteExtraneous->checkState() == Qt::Checked;
		this->model->schedule( getSelectedFilesAndDirs(), this->includePatternList, this->excludePatternList, ScheduledTask(wd, time), setDeleteFlag );
	}
	else if ( this->radioButtonMinutesAfterBooting->isChecked() )
	{
		int delay = this->spinBoxMinutesAfterBooting->value();
		bool setDeleteFlag = this->checkBoxDeleteExtraneous->checkState() == Qt::Checked;
		this->model->schedule( getSelectedFilesAndDirs(), this->includePatternList, this->excludePatternList, ScheduledTask(delay), setDeleteFlag );
	}
	else if ( this->radioButtonNoSchedule->isChecked() )
	{
		this->model->schedule( getSelectedFilesAndDirs(), this->includePatternList, this->excludePatternList, ScheduledTask(), FALSE );
	}
	qDebug() << "BackupForm::schedule(): emit updateOverviewFormScheduleInfo()";
	emit updateOverviewFormScheduleInfo();
}


void BackupForm::on_btnBackup_pressed()
{
	if ( this->localDirModel->getSelectionRules().size() == 0 )
	{
		QMessageBox::information( this, tr( "Empty backup list" ), tr( "No items have been selected for backup" ));
		return;
	}
	bool deleteExtraneous = this->checkBoxDeleteExtraneous->checkState() == Qt::Checked;
	this->model->showProgressDialogSlot( tr( "Backup" ) );
	qDebug() << "BackupForm::on_btnBackup_pressed()" << this->model->getLocalDirModel()->getSelectionRules();
	this->model->backup( this->model->getLocalDirModel()->getSelectionRules(), deleteExtraneous, false );
	//this->model->backup( backupItems, includePatternList, excludePatternList, deleteExtraneous, false );
	emit updateOverviewFormLastBackupsInfo();
}

void BackupForm::on_btnEditInclude_pressed()
{
	PatternDialog patternDialog( tr( "Include Pattern" ), tr( "Edit include patterns" ), this->includePatternList );
	QObject::connect( &patternDialog, SIGNAL( getPatternList( QStringList ) ),
						this, SLOT ( setIncludePatternList( QStringList ) ) );
	patternDialog.exec();
	QObject::disconnect( &patternDialog, SIGNAL( getPatternList( QStringList ) ),
						this, SLOT ( setIncludePatternList( QStringList ) ) );
	this->lineEditIncludePattern->setText( patternListToString( this->includePatternList ) );
	Settings::getInstance()->saveIncludePatternList( includePatternList );
}

void BackupForm::on_btnEditExclude_pressed()
{
	PatternDialog patternDialog( tr( "Exclude Pattern" ), tr( "Edit exclude patterns" ), this->excludePatternList );
	QObject::connect( &patternDialog, SIGNAL( getPatternList( QStringList ) ),
						this, SLOT ( setExcludePatternList( QStringList ) ) );
	patternDialog.exec();
	QObject::disconnect( &patternDialog, SIGNAL( getPatternList( QStringList ) ),
						this, SLOT ( setExcludePatternList( QStringList ) ) );
	this->lineEditExcludePattern->setText( patternListToString( this->excludePatternList ) );
	Settings::getInstance()->saveExcludePatternList( excludePatternList );
}

void BackupForm::setIncludePatternList( const QStringList& includePatternList )
{
	this->includePatternList = includePatternList;
	this->lineEditIncludePattern->setText( patternListToString( this->includePatternList ) );
}

void BackupForm::setExcludePatternList( const QStringList& excludePatternList )
{
	this->excludePatternList = excludePatternList;
	this->lineEditExcludePattern->setText( patternListToString( this->excludePatternList ) );
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

void BackupForm::setDetailsVisible( bool visible )
{
	this->groupBoxPatterns->setVisible( visible );
	this->groupBoxDeletion->setVisible( visible );

	if ( visible )
	{
		this->btnDetails->setText( tr( "L&ess..." ) );
	}
	else
	{
		this->btnDetails->setText( tr( "&More..." ) );
	}
	detailsVisible = visible;
}

void BackupForm::on_btnDetails_pressed()
{
	setDetailsVisible( !detailsVisible );
}

void BackupForm::refreshLocalDirModel()
{
	this->localDirModel->refresh();
}

void BackupForm::on_btnRefresh_pressed()
{
	refreshLocalDirModel();
}
