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

#include <QFileDialog>
#include <QDebug>

#include "gui/restore_form.hh"
#include "settings/settings.hh"
#include "settings/platform.hh"
#include "model/dir_tree_item.hh"
#include "model/main_model.hh"
#include "model/remote_dir_model.hh"

RestoreForm::~RestoreForm()
{
	QObject::disconnect( this->comboBoxPrefixes, SIGNAL( currentIndexChanged( int ) ),
					 this, SLOT( initRestoreNames() ) );
}

RestoreForm::RestoreForm ( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );
	initialized = false;

	this->lastRestoreDestination = "";
	this->model = model;
	this->labelPrefixes->setText( "" );
	this->labelBackupNames->setText( "" );
	QObject::connect( this->comboBoxPrefixes, SIGNAL( currentIndexChanged( int ) ),
					  this, SLOT( initRestoreNames() ) );
}

bool RestoreForm::isInitialized()
{
	return initialized;
}

void RestoreForm::initPrefixes()
{
	QObject::disconnect( this->comboBoxPrefixes, SIGNAL( currentIndexChanged( int ) ),
						this, SLOT( initRestoreNames() ) );

	this->model->showProgressDialogSlot( tr( "Downloading prefixes" ) );
	this->model->clearRemoteDirModel();

	Settings* settings = Settings::getInstance();

	// get prefixes
	this->comboBoxPrefixes->clear();
	QStringList prefixes = this->model->getPrefixes();

	if( prefixes.size() > 0 )
	{
		setPrefixSelectionDisabled( false );
		setBackupSelectionDisabled( false );
		for( int i=0; i<prefixes.size(); i++ )
		{
			QString prefix = prefixes.at( i );
			this->comboBoxPrefixes->addItem( prefix );
			if( prefix == settings->getBackupPrefix() )
			{
				// pre-select the default prefix
				this->comboBoxPrefixes->setCurrentIndex( i );
			}
		}
		this->model->closeProgressDialogSlot();
		initRestoreNames();
		QObject::connect( this->comboBoxPrefixes, SIGNAL( currentIndexChanged( int ) ),
						  this, SLOT( initRestoreNames() ) );
	}
	else
	{
		this->model->closeProgressDialogSlot();
		setPrefixSelectionDisabled( true );
		setBackupSelectionDisabled( true );
	}
	initialized = true;
}

void RestoreForm::initRestoreNames()
{
	qDebug() << "RestoreForm::initRestoreNames()" << "prefix:" << this->comboBoxPrefixes->currentText();
	this->model->showProgressDialogSlot( tr( "Downloading restore meta data" ) );
	QObject::disconnect( this->comboBoxBackupNames, SIGNAL( currentIndexChanged( int ) ),
						 this, SLOT( populateFilesAndFoldersTree() ) );

	// settings->saveBackupPrefix( this->comboBoxPrefixes->currentText() );
	this->comboBoxBackupNames->clear();
	QList<RestoreName> restoreNames = this->model->getRestoreNames(this->comboBoxPrefixes->currentText());

	if ( restoreNames.size() > 0 )
	{
		setBackupSelectionDisabled( false );
		foreach( RestoreName restoreName, restoreNames )
		{
			if( restoreName.getDate() == QDate::currentDate() )
			{
				this->comboBoxBackupNames->addItem( tr( "Today" ), restoreName.getAbsoluteDirName() );
			}
			else
			{
				this->comboBoxBackupNames->addItem( restoreName.getText(), restoreName.getAbsoluteDirName() );
			}
		}
		populateFilesAndFoldersTree();
		QObject::connect( this->comboBoxBackupNames, SIGNAL( currentIndexChanged( int ) ),
						  this, SLOT( populateFilesAndFoldersTree() ) );
	}
	else
	{
		setBackupSelectionDisabled( true );
	}
	this->model->closeProgressDialogSlot();
	this->expandTreePart();
}

void RestoreForm::setPrefixSelectionDisabled( const bool& disable )
{
	this->comboBoxPrefixes->setDisabled( disable );
	if( disable )
	{
		this->labelPrefixes->setText( tr( "No prefixes found" ) );
	}
	else
	{
		this->labelPrefixes->setText( "" );
	}
}

void RestoreForm::setBackupSelectionDisabled( const bool& disable )
{
	// this->comboBoxBackupNames->setDisabled( disable );
	this->comboBoxBackupNames->setVisible( !disable );
	this->labelBackupNames->setText( disable ? tr( "Nothing found to restore" ) : "" );
	this->radioButtonFull->setDisabled( disable );
	this->radioButtonCustom->setDisabled( disable );
	this->btnBrowseAndRestore->setDisabled( disable );
	this->btnRestore->setDisabled( disable );
	this->setRestoreTreeDisabled( disable );
	if (disable) this->treeViewFilesAndFolders->setModel( 0 );
}

void RestoreForm::setRestoreTreeDisabled( const bool& disable )
{
	this->treeViewFilesAndFolders->setDisabled( disable );
	// this->treeViewFilesAndFolders->setModel( disable ? 0 : this->model->getCurrentRemoteDirModel() );
}

void RestoreForm::populateFilesAndFoldersTree()
{
	qDebug() << "RestoreForm::populateFilesAndFoldersTree()";
	QApplication::setOverrideCursor( QCursor(Qt::WaitCursor ) );
	this->refreshRemoteDirModel();
	this->treeViewFilesAndFolders->setSelectionMode( QAbstractItemView::NoSelection );

	this->expandTreePart();
	QApplication::restoreOverrideCursor();
}

void RestoreForm::expandTreePart()
{
	//expand the first visible root item
	int nLevels = 2;
	int iLevel = 0;
	QStandardItemModel* remoteDirModel = this->model->getCurrentRemoteDirModel();
	if (remoteDirModel != 0) {
		QStandardItem* dirItem = remoteDirModel->invisibleRootItem();
		while ( iLevel < nLevels && dirItem->hasChildren())
		{
			this->treeViewFilesAndFolders->setExpanded( remoteDirModel->indexFromItem( dirItem ), true );
			dirItem = dirItem->child(0);
			iLevel++;
		}
	}
}

void RestoreForm::refreshRemoteDirModel()
{
	this->model->clearRemoteDirModel();
	QString currentBackupName = this->comboBoxBackupNames->itemData( this->comboBoxBackupNames->currentIndex() ).toString();
	qDebug() << "RestoreForm::refreshRemoteDirModel: currentBackupName=" << currentBackupName;
	QString currentPrefix = this->comboBoxPrefixes->currentText();
	this->treeViewFilesAndFolders->setModel( this->model->loadRemoteDirModel( currentPrefix, currentBackupName ) );
	this->treeViewFilesAndFolders->repaint();
}

void RestoreForm::on_radioButtonFull_clicked()
{
	this->setRestoreTreeDisabled( true );
}

void RestoreForm::on_radioButtonCustom_clicked()
{
	this->setRestoreTreeDisabled( false );
}

void RestoreForm::on_btnRefresh_clicked()
{
	initPrefixes();
}

bool RestoreForm::browseForDestinationFolder(QString& destination)
{
	QFileDialog fileDialog( this );
	fileDialog.setFileMode( QFileDialog::DirectoryOnly );
	fileDialog.setWindowTitle( QObject::tr("Restore destination folder") );
	if (lastRestoreDestination != "" && QFileInfo(lastRestoreDestination).exists()) {
		fileDialog.setDirectory( lastRestoreDestination );
	}
	bool retVal = (fileDialog.exec() == QDialog::Accepted);
	qDebug() << "retVal=" << (retVal ? "true" : "false");
	if ( retVal )
	{
		QStringList selectedDir = fileDialog.selectedFiles();
		if ( selectedDir.size() != 1 )
		{
			destination = "";
			return retVal;
		}
		destination = selectedDir.first();
		return retVal;
	}
	return retVal;
}

void RestoreForm::on_btnBrowseAndRestore_clicked()
{
	startRestore(this->radioButtonFull->isChecked(), "");
}

void RestoreForm::on_btnRestore_clicked()
{
    startRestore(this->radioButtonFull->isChecked(), Platform::ROOT_PREFIX); 
}

void RestoreForm::startRestore(bool isFullRestore, QString destination)
{
	// validate user's selection
	this->lastRestoreDestination = destination;
	if ( !this->treeViewFilesAndFolders->selectionModel() )
	{
		this->model->showInformationMessage( tr( "You can not start a restore because no backup is available." ) );
		return;
	}
	if ( this->comboBoxBackupNames->currentIndex() == -1 )
	{
		this->model->showInformationMessage( tr( "No backup has been selected." ) );
		return;
	}
	QString selectedBackupName = this->comboBoxBackupNames->itemData( this->comboBoxBackupNames->currentIndex() ).toString();
	QString selectedBackupPrefix = this->comboBoxPrefixes->currentText();
	if ( !isFullRestore && (this->model->getCurrentRemoteDirModel()->getSelectionRules().size() == 0) )
	{
		this->model->showInformationMessage( tr( "No files and/or directories has been selected." ) );
		return;
	}
	bool selectionAborted = false;
	while ( (destination == "" || !QFileInfo( destination ).exists()) && !(selectionAborted = !browseForDestinationFolder(destination)) && destination == "" )	{
		this->model->showInformationMessage( "No valid destination folder has been selected.\nPlease select a valid restore destination." );
	}
	if (selectionAborted) {
		return;
	}
	if ( destination != "/" && !QFileInfo( destination ).exists() )
	{
		this->model->showInformationMessage( tr( "Specific destination path is not valid" ) );
		return;
	}

	destination = QDir::fromNativeSeparators( destination );
	if ( isFullRestore ) {
		// full restore
		this->model->showProgressDialogSlot( tr( "Full restore" ) );
		this->model->fullRestore( selectedBackupPrefix, selectedBackupName, destination );
	} else {
		// custom restore
		this->model->showProgressDialogSlot( tr( "Custom restore" ) );
		BackupSelectionHash selectionRules = this->model->getCurrentRemoteDirModel()->getSelectionRules();
		this->model->customRestore( selectionRules, selectedBackupPrefix, selectedBackupName, destination );
	}
}
