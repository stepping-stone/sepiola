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

#ifndef TRAFFIC_PROGRESS_DIALOG_HH
#define TRAFFIC_PROGRESS_DIALOG_HH

#include <QDialog>
#include <QTime>

#include "ui_traffic_progress_dialog.h"

/**
 * The OutputDialog class provides a dialog supporting text output
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class TrafficProgressDialog : public QDialog, private Ui::TrafficProgressDialog
{
	Q_OBJECT

public:

	/**
	 * Constructs a OutputDialog with a given title
	 */
	TrafficProgressDialog( const QString& title );

	/**
	 * Destroys the OutputDialog
	 */
	virtual ~TrafficProgressDialog();

	/**
	 * Appends the given info to the dialog
	 * @param info text to append
	 */
	void appendInfo( const QString& info );
	
	/**
	 * Displayes the given filename as info below the progressbar
	 * @param filename filename to display
	 */
	void displayInfoFilename(const QString& filename);

	/**
	 * Appends the given error to the dialog
	 * @param error text to append
	 */
	void appendError( const QString& error );

	/**
	 * Enables the finish button
	 */
	void finished();

	void closeEvent( QCloseEvent * event );

signals:
	bool abort();

private slots:
	void on_btnCancel_pressed();
	void flushCache();

private:
	bool isErrorVisible;
	QTime lastUpdate;
	QString outputCache;
};

#endif // TRAFFIC_PROGRESS_DIALOG_HH
