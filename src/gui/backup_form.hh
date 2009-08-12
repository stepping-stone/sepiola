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

#ifndef BACKUP_FORM_HH
#define BACKUP_FORM_HH

#include "ui_backup_form.h"

#include "model/main_model.hh"
#include "model/local_dir_model.hh"

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
	void on_btnRefresh_pressed();
	void on_btnSchedule_pressed();
	void on_btnBackup_pressed();
	void refreshLocalDirModel();
	void schedule();
	
signals:	
	void updateOverviewFormScheduleInfo();
	void updateOverviewFormLastBackupsInfo();

private:
	void expandSelectedBranches();
	
	MainModel* model;
	LocalDirModel* localDirModel;
	bool detailsVisible;
};

#endif
