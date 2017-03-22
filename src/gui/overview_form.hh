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

#ifndef OVERVIEW_FORM_HH
#define OVERVIEW_FORM_HH

#include <QWidget>

#include "ui_overview_form.h"

// Forward declarations
class MainModel;
class SpaceUsageModel;

/**
 * The OutputDialog class provides a dialog supporting text output
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class OverviewForm : public QWidget, private Ui::OverviewForm
{
	Q_OBJECT
public:
	static const int MAX_LAST_BACKUP_INFOS = 5;

	/**
	 * Constructs a OutputDialog with a given title
	 */
	OverviewForm( QWidget *parent, MainModel *model );

	/**
	 * Destroys the OutputDialog
	 */
	virtual ~OverviewForm();

public slots:
	/**
 	 * Refreshes the graphical quota-statistics
	 */
	void refreshSpaceStatistic();

	/**
	 * Refreshes the overview of the last n backups
	 */
	void refreshLastBackupsOverview();

	/**
	 * Refreshes the overview of the scheduled backup
	 */
	void refreshScheduleOverview();

private slots:
	void on_btnBackupNow_clicked();

signals:
	void startBackupNow();

private:
	MainModel* model;
    SpaceUsageModel* spaceUsageModel;
};

#endif
