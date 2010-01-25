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
#include "utils/string_utils.hh"
#include "model/backup_task.hh"

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
	 * Displayes the given filename as info below the progressbar
	 * @param filename filename to display
	 */
	void displayFilename(QLabel* lbl, const QString& filename);


	/**
	 * Enables the finish button
	 */
	void finished();

	void closeEvent( QCloseEvent * event );

	void setInfo(QLabel* lbl_label, QLabel* lpl_value, QPair<QString,QString> label_and_value);
	void setInfoLine(QLabel* lbl_lbl, QLabel* val_lbl, QPair<QString,QString> label_and_value);
	void setInfoLine1(QPair<QString,QString> label_and_value);
	void setInfoLine2(QPair<QString,QString> label_and_value);
	void setInfoLine3(QPair<QString,QString> label_and_value);
	void setInfoLine4(QPair<QString,QString> label_and_value);
	void setInfoLines(QList< QPair<QString,QString> > label_and_values);

signals:
	bool abort();

public slots:
	/**
	 * Appends the given info to the dialog
	 * @param info text to append
	 */
	void appendInfo( const QString& info );
	/**
	 * Appends the given error to the dialog
	 * @param error text to append
	 */
	void appendError( const QString& error );
	void updateProgress( const QString& taskText, float percentFinished, const QDateTime& timeRemaining, StringPairList infos );
	void showFinalStatus(ConstUtils::StatusEnum status);

private slots:
	void on_btnCancel_pressed();
	void on_btnShowHideDetails_pressed();
	void flushCache();
	void flushInfoLines();

private:
	bool isErrorVisible;
	bool isInfoVisible;
	QDateTime lastUpdateTextLog;
	QDateTime lastUpdateTextInfo;
	QString outputCache;
	StringPairList lastInfos;
};

#endif // TRAFFIC_PROGRESS_DIALOG_HH
