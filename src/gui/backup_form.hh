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

#ifndef BACKUP_FORM_HH
#define BACKUP_FORM_HH

#include "ui_backup_form.h"

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTime>

// Forward declarations
class MainModel;
class LocalDirModel;

/**
 * The BackupForm class is the backup user interface.
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class BackupForm : public QWidget, private Ui::BackupForm
{
	Q_OBJECT
public:

	/**
	 * Constructs a BackupForm which is a child of parent with a given model
	 */
	BackupForm ( QWidget *parent, MainModel *model );

	/**
	 * Destroys the BackupForm
	 */
	virtual ~BackupForm();

private:
	QStringList getSelectedFilesAndDirs();
	QString patternListToString( QStringList patternList );

private slots:
	void on_btnRefresh_clicked();
	void runBackupNow();
	void save();
	void reset();
	void reload();
	void on_radioButtonNoSchedule_clicked();
	void on_radioButtonMinutesAfterBooting_clicked();
	void on_radioButtonDaily_clicked();
	void refreshLocalDirModel();
	void schedule();

signals:
	void updateOverviewFormScheduleInfo();
	void updateOverviewFormLastBackupsInfo();

private:
	void disableScheduleOptions();
	void expandSelectedBranches();

	static const QTime default_schedule_time;
	static const int default_schedule_minutesAfterStartup;
	MainModel* model;
	LocalDirModel* localDirModel;
};

#endif
