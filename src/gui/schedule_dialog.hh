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

#ifndef SCHEDULE_DIALOG_HH
#define SCHEDULE_DIALOG_HH

#include <QDialog>

#include "ui_schedule_dialog.h"
#include "model/main_model.hh"

/**
 * The ScheduleDialog class provides a dialog for scheduling a backup job
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: dsydler $ $Date: 2008/07/04 08:46:21 $ $Revision: 1.12 $
 */
class ScheduleDialog : public QDialog, private Ui::ScheduleDialog
{
	Q_OBJECT
	
public:
	/**
	 * Constructs a ScheduleDialog
	 * @param model The main model
	 * @param backupItems List of items to backup
	 * @param includePatternList Pattern list for include files 
	 * @param excludePatternList Pattern list for exclude files
	 */
	ScheduleDialog( MainModel* model, const QStringList& backupItems, const QStringList& includePatternList, const QStringList& excludePatternList );
	
	/**
	 * Destroys the ScheduleDialog
	 */
	virtual ~ScheduleDialog();

private:
	MainModel* model;
	QStringList backupItems;
	QStringList includePatternList;
	QStringList excludePatternList;

private slots:
	void schedule();
};

#endif
