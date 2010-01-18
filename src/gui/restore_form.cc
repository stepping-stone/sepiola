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

#include <QFileDialog>
#include <QDebug>

#include "gui/restore_form.hh"
#include "model/dir_tree_item.hh"

RestoreForm::~RestoreForm()
{
	QObject::disconnect( this->comboBoxPrefixes, SIGNAL( currentIndexChanged( int ) ),
					 this, SLOT( initRestoreNames() ) );
}

RestoreForm::RestoreForm ( QWidget *parent, MainModel *model ) : QWidget ( parent )
{
	setupUi ( this );
	initialized = false;
	QButtonGroup* restoreTypGroup = new QButtonGroup( this );
	restoreTypGroup->addButton( this->radioButtonFull );
	restoreTypGroup->addButton( this->radioButtonCustom );
	this->radioButtonCustom->setChecked( true );

	QButtonGroup* restoreToGroup = new QButtonGroup( this )	;
	restoreToGroup->addButton( this->radioButtonOrigin );
	restoreToGroup->addButton( this->radioButtonSpecific );
	this->radioButtonSpecific->setChecked( true );

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
	Settings* settings = Settings::getInstance();
	QObject::disconnect( this->comboBoxBackupNames, SIGNAL( currentIndexChanged( int ) ),
						 this, SLOT( populateFilesAndFoldersTree() ) );

	settings->saveBackupPrefix( this->comboBoxPrefixes->currentText() );
	this->comboBoxBackupNames->clear();
	QList<RestoreName> restoreNames = this->model->getRestoreNames();

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
	this->comboBoxBackupNames->setDisabled( disable );
	this->radioButtonFull->setDisabled( disable );
	this->radioButtonCustom->setDisabled( disable );
	this->groupBoxCustom->setDisabled( disable );
	this->radioButtonOrigin->setDisabled( disable );
	this->radioButtonSpecific->setDisabled( disable );
	this->lineEditPath->setDisabled( disable );
	this->btnBrowse->setDisabled( disable );
	this->btnRestore->setDisabled( disable );

	if( disable )
	{
		this->labelBackupNames->setText( tr( "Nothing found to restore" ) );
	}
	else
	{
		this->labelBackupNames->setText( "" );
	}
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
	QString currentPrefix = this->comboBoxPrefixes->currentText();
	this->treeViewFilesAndFolders->setModel( this->model->loadRemoteDirModel( currentPrefix, currentBackupName ) );
	this->treeViewFilesAndFolders->repaint();
}

void RestoreForm::on_btnRefresh_pressed()
{
	initPrefixes();
}

void RestoreForm::on_btnBrowse_pressed()
{
	QFileDialog fileDialog( this );
	fileDialog.setFileMode( QFileDialog::DirectoryOnly );
	if ( fileDialog.exec() )
	{
		QStringList selectedDir = fileDialog.selectedFiles();
		if ( selectedDir.size() != 1 )
		{
			return;
		}
		this->lineEditPath->setText( selectedDir.first() );
	}
}

void RestoreForm::on_btnRestore_pressed()
{
	// validate user's selection
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
	QString currentBackupName = this->comboBoxBackupNames->itemData( this->comboBoxBackupNames->currentIndex() ).toString();
	QString currentPrefix = this->comboBoxPrefixes->currentText();
	if ( this->radioButtonCustom->isChecked() && (this->model->getCurrentRemoteDirModel()->getSelectionRules().size() == 0) )
	{
		this->model->showInformationMessage( tr( "No files and/or directories has been selected." ) );
		return;
	}


	if ( this->radioButtonSpecific->isChecked() && !QFileInfo( this->lineEditPath->text() ).exists() )
	{
		this->model->showInformationMessage( tr( "Specific destination path is not valid" ) );
		return;
	}

	QString destination;
	if ( this->radioButtonOrigin->isChecked() )
	{
		destination = "/";
	}
	else
	{
		destination = this->lineEditPath->text();
	}
	destination = QDir::fromNativeSeparators( destination );
	if ( this->radioButtonFull->isChecked() )
	{
		// full restore
		this->model->showProgressDialogSlot( tr( "Full restore" ) );
		this->model->fullRestore( currentBackupName, destination );
	}
	else
	{
		// custom restore
		this->model->showProgressDialogSlot( tr( "Custom restore" ) );
		BackupSelectionHash selectionRules = this->model->getCurrentRemoteDirModel()->getSelectionRules();
		this->model->customRestore( this->model->getCurrentRemoteDirModel(), selectionRules, currentBackupName, destination );
	}
}
