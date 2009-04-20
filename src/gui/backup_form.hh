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

public slots:

	/**
	 * Slot for setting the include pattern list
	 * @param includePatternList list of patterns to set
	 */
	void setIncludePatternList( const QStringList& includePatternList );

	/**
	 * Slot for setting the exclude pattern list
	 * @param excludePatternList list of patterns to set
	 */
	void setExcludePatternList( const QStringList& excludePatternList );
	
private slots:
	void on_btnRefresh_pressed();
	void on_btnSchedule_pressed();
	void on_btnBackup_pressed();
	void on_btnEditInclude_pressed();
	void on_btnEditExclude_pressed();
	void on_btnDetails_pressed();
	void setDetailsVisible( bool visible );
	void refreshLocalDirModel();
	void schedule();

private:
	MainModel* model;
	QDirModel* localDirModel;
	QStringList includePatternList;
	QStringList excludePatternList;
	bool detailsVisible;
};

#endif
