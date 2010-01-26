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

#ifndef RESTORE_FORM_HH
#define RESTORE_FORM_HH

#include <QWidget>

#include "ui_restore_form.h"

#include "model/main_model.hh"


/**
 * The RestoreForm class is the restore user interface.
 * @author Reto Aebersold, aebersold@puzzle.ch
 */
class RestoreForm : public QWidget, private Ui::RestoreForm
{
	Q_OBJECT
public:
	/**
	 * Constructs a RestoreForm which is a child of parent with a given model
	 */
	RestoreForm ( QWidget* parent, MainModel* model );

	/**
	 * Destroys the RestoreForm
	 */
	virtual ~RestoreForm();

	/**
	 * Returns whether the restore list has been initialized
	 */
	bool isInitialized();

	/**
	 * Initializes the prefix list
	 */
	void initPrefixes();
	void expandTreePart();
	void expandSelectedBranches();

	void refreshRemoteDirModel();
	RemoteDirModel* getCurrentRemoteDirModel();

private slots:
	void initRestoreNames();
	void on_btnBrowseAndRestore_clicked();
	void on_btnRestore_clicked();
	void on_btnRefresh_clicked();
	void on_radioButtonFull_clicked();
	void on_radioButtonCustom_clicked();
	void populateFilesAndFoldersTree();

private:
	MainModel* model;
	bool initialized;
	QString lastRestoreDestination;

	bool browseForDestinationFolder(QString& destination);
	void startRestore(bool isFullRestore, QString destination);
	void setPrefixSelectionDisabled( const bool& disable );
	void setBackupSelectionDisabled( const bool& disable );
	void setRestoreTreeDisabled( const bool& disable );
};

#endif
