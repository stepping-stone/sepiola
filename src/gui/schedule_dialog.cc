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

#include "schedule_dialog.hh"

ScheduleDialog::ScheduleDialog( MainModel* model, const QStringList& backupItems, const QStringList& includePatternList, const QStringList& excludePatternList )
{
	setupUi ( this );

	this->model = model;

	// disable option "minutes after booting" if the scheduler does not support it
	if ( !this->model->isSchedulingOnStartSupported() )
	{
		this->radioButtonMinutesAfterBooting->setEnabled( false );
		this->lineEditMinutesAfterBooting->setEnabled( false );
	}

	// change the label of ok and cancel button to schedule and cancel
	QList<QAbstractButton*> buttons = this->buttonBox->buttons();
	foreach( QAbstractButton* button, buttons )
	{
		switch( this->buttonBox->buttonRole( button ) )
		{
			case QDialogButtonBox::AcceptRole:
				button->setText( tr( "&Schedule" ) );
				break;
			case QDialogButtonBox::RejectRole:
				button->setText( tr( "&Cancel" ) );
				break;
			default:
				qWarning() << "Not expected button found";
		}
	}

	QObject::connect( this->buttonBox, SIGNAL( accepted() ),
						this, SLOT( schedule() ) );

	this->backupItems = backupItems;
	this->includePatternList = includePatternList;
	this->excludePatternList = excludePatternList;
}

ScheduleDialog::~ScheduleDialog()
{
	QObject::disconnect( this->buttonBox, SIGNAL( accepted() ),
							 this, SLOT( schedule() ) );
}

void ScheduleDialog::schedule()
{
	if ( this->radioButtonDaily->isChecked() )
	{
		QTime time = this->timeEditTime->time();
		bool days[7] = {
			this->checkBoxMonday->checkState() == Qt::Checked,
			this->checkBoxTuesday->checkState() == Qt::Checked,
			this->checkBoxWednesday->checkState() == Qt::Checked,
			this->checkBoxThursday->checkState() == Qt::Checked,
			this->checkBoxFriday->checkState() == Qt::Checked,
			this->checkBoxSaturday->checkState() == Qt::Checked,
			this->checkBoxSunday->checkState() == Qt::Checked,
		};

		bool validSelection = false;
		for ( int i=0; i<7; i++ )
		{
			if ( days[i] )
			{
				validSelection = true;
				break;
			}
		}

		if ( !validSelection )
		{
			this->model->infoDialog( tr( "Check at least one day" ) );
			return;
		}
		bool setDeleteFlag = this->checkBoxDeleteExtraneous->checkState() == Qt::Checked;
		this->model->schedule( this->backupItems, this->includePatternList, this->excludePatternList, time, days, setDeleteFlag );
	}
	else if ( this->radioButtonMinutesAfterBooting->isChecked() )
	{
		bool ok;
		int delay = this->lineEditMinutesAfterBooting->text().toInt( &ok );
		if ( !ok )
		{
			QMessageBox::information( this,
										tr( "Value not valid" ),
										tr( "Value for \"minutes after booting\" is not valid" )
									);
			return;
		}
		bool setDeleteFlag = this->checkBoxDeleteExtraneous->checkState() == Qt::Checked;
		this->model->schedule( this->backupItems, this->includePatternList, this->excludePatternList, delay, setDeleteFlag );
	}
	this->done(0);
}
