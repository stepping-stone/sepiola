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

#ifndef LOGFILE_FORM_HH
#define LOGFILE_FORM_HH

#include "ui_logfile_form.h"

#include "model/main_model.hh"

/**
 * The LogfileDialog class provides a dialog showing the log file
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class LogfileForm : public QWidget, private Ui::LogfileForm
{
	Q_OBJECT
	
public:

	/**
	 * Constructs a LogfileForm which is a child of parent with a given model
	 */
	LogfileForm( QWidget *parent, MainModel *model );
	
	/**
	 * Destroys the LogfileForm
	 */
	virtual ~LogfileForm();
	
	/**
	 * Appends new log lines
	 * @param logfileLines lines to set
	 */
	void appendLines( const QStringList& logfileLines );
};

#endif
