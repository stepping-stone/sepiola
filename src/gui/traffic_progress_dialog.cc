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

#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QFileInfo>
#include <QMovie>
#include <QMessageBox>

#include "gui/traffic_progress_dialog.hh"
#include "settings/settings.hh"
#include "settings/platform.hh"
#include "utils/debug_timer.hh"


TrafficProgressDialog::TrafficProgressDialog( const QString& title )
{
	setupUi ( this );
	setWindowTitle( title );
	setAttribute( Qt::WA_DeleteOnClose );

	this->labelTitle->setText( title );
	this->labelError->setVisible( false );
	this->textEditError->setVisible( false );
	this->isErrorVisible = false;

	this->labelInfo->setVisible( !Settings::SHOW_PROGRESS );
	this->textEditInfo->setVisible( !Settings::SHOW_PROGRESS );
	this->isInfoVisible = !Settings::SHOW_PROGRESS;

	this->btnClose->setEnabled( false );
	this->btnClose->setVisible( false );
	this->btnPause->setVisible( false );

	this->groupBoxFinalStatus->setVisible( false );
	this->lastUpdateTextLog = QDateTime::currentDateTime();
	this->lastUpdateTextInfo = QDateTime::currentDateTime();

	this->label_animatedSepiolaLogo->setVisible(false);
	/*QMovie* movie = new QMovie(":/main/sepiola_logo_anim.gif");
	this->label_animatedSepiolaLogo->setMovie(movie);
	this->repaint();
	movie->start();*/

	// $ERASE_PROGRESS: hide all progress information
	this->progressBar->setVisible(Settings::SHOW_PROGRESS);
	this->groupBoxInfos->setVisible(Settings::SHOW_PROGRESS);
	this->btnShowHideDetails->setVisible(Settings::SHOW_PROGRESS);
}

TrafficProgressDialog::~TrafficProgressDialog()
{
}

void TrafficProgressDialog::appendInfo( const QString& info )
{
	const int msecs_to_wait_for_flush = 500;
	QDateTime currentTime = QDateTime::currentDateTime();
	this->outputCache.append( info );
    this->outputCache.append( Platform::EOL_CHARACTER );
	if ( this->lastUpdateTextLog.addMSecs(msecs_to_wait_for_flush) <= currentTime )
	{
		// update, but only after a second, such that following messages ariving in the next second are also flushed
		this->lastUpdateTextLog = currentTime;
		QTimer::singleShot(msecs_to_wait_for_flush, this, SLOT(flushCache()));
	}
}

void TrafficProgressDialog::flushCache()
{
	this->textEditInfo->append( this->outputCache  );
	this->outputCache.clear();
}

void TrafficProgressDialog::appendError( const QString& error )
{
	if ( !this->isErrorVisible )
	{
		this->labelError->setVisible( true );
		this->textEditError->setVisible( true );
		this->isErrorVisible = true;
	}
	this->textEditError->append( error  );
}

void TrafficProgressDialog::on_btnCancel_clicked()
{
	qDebug() << "BackupProgress::on_btnCancel_clicked(): switching Button-visibility";
	int answer = QMessageBox::question(this, tr("Cancel"), tr("Do you really want to cancel"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (answer == QMessageBox::Yes) {
		emit abort();
		this->btnClose->setEnabled(true);
		this->btnClose->setVisible(true);
		this->btnCancel->setEnabled(false);
		this->btnCancel->setVisible(false);
		this->progressBar->setValue(this->progressBar->maximum());
	}
}

void TrafficProgressDialog::on_btnShowHideDetails_clicked()
{
	qDebug() << "TrafficProgressDialog::on_btnShowHideDetails_clicked(): info- and error-textfield visibility";
	bool newVis = !(this->isErrorVisible || this->isInfoVisible);
	this->btnShowHideDetails->setText ( newVis ? tr("Hide details") : tr("Show details") );
	this->labelInfo->setVisible( newVis );
	this->textEditInfo->setVisible( newVis );
	this->isInfoVisible = newVis;
	newVis = newVis && (this->textEditError->toPlainText().length()>0);
	this->labelError->setVisible( newVis );
	this->textEditError->setVisible( newVis );
	this->isErrorVisible = newVis;
}

void TrafficProgressDialog::finished()
{
	flushCache();
	this->progressBar->setValue(this->progressBar->maximum());
	this->btnClose->setEnabled( true );
	this->btnClose->setVisible( true );
	this->btnCancel->setEnabled( false );
	this->btnCancel->setVisible( false );
	this->textEditInfo->ensureCursorVisible();
}

void TrafficProgressDialog::showFinalStatus(ConstUtils::StatusEnum status)
{
	qDebug() << "TrafficProgressDialog::showFinalStatus(" << status << ")";
	QString html_templ = "<font style=\"color:rgb(%2,%3,%4)\">%1</font>";
	QString style_templ = "color: rgb(%1, %2, %3);";
	QImage img;
	QColor text_color;
	QString statusText;
	switch (status)
	{
		case ConstUtils::STATUS_OK:
			img.load( ":/main/sign_ok.svg" );
			statusText = tr("successfully finished");
			text_color = QColor(0,191,0);
			break;
		case ConstUtils::STATUS_WARNING:
			img.load( ":/main/sign_warning.svg" );
			statusText = tr("finished with warnings");
			text_color = QColor(255,191,0);
			break;
		case ConstUtils::STATUS_ERROR:
		default:
			img.load( ":/main/sign_error.svg" );
			statusText = tr("failed");
			text_color = QColor(223,0,0);
			break;
	}
	QString rNr, gNr, bNr;
	this->groupBoxFinalStatus->setVisible( true );
	this->label_iconFinalStatus->setPixmap( QPixmap::fromImage(img) );
	this->label_finalStatus->setText( html_templ.arg(statusText, rNr.setNum(text_color.red()), gNr.setNum(text_color.green()), bNr.setNum(text_color.blue())) );
	this->label_finalStatus->setStyleSheet( style_templ.arg(rNr.setNum(text_color.red()), gNr.setNum(text_color.green()), bNr.setNum(text_color.blue())) );
}


void TrafficProgressDialog::closeEvent( QCloseEvent * event )
{
	if (!this->btnClose->isEnabled()) event->ignore();
}

void TrafficProgressDialog::displayFilename(QLabel* lbl, const QString& filename)
{
	//qDebug() << "TrafficProgressDialog::displayInfoFilename(" << filename << ")";
	//int curWidth = lbl->fontMetrics().width( filename );
	//int maxWidth = (int)(this->width() - lbl->x()*0.9f);
	int maxChar_approx = 50;//(int)(maxWidth * filename.length() / curWidth);
	lbl->setText(StringUtils::filenameShrink(filename, maxChar_approx));
}

void TrafficProgressDialog::setInfo(QLabel* lbl_label, QLabel* lpl_value, QPair<QString,QString> label_and_value)
{
	lbl_label->setText(label_and_value.first);
	if (StringUtils::isFilename(label_and_value.second)) {
		displayFilename(lpl_value, label_and_value.second);
	} else {
		lpl_value->setText(label_and_value.second);
	}
}

void TrafficProgressDialog::setInfoLine(QLabel* lbl_lbl, QLabel* val_lbl, QPair<QString,QString> label_and_value)
{
	lbl_lbl->setText(label_and_value.first);
	if (StringUtils::isFilename(label_and_value.second))
		displayFilename(val_lbl, label_and_value.second);
	else
		val_lbl->setText(label_and_value.second);
}
void TrafficProgressDialog::setInfoLine1(QPair<QString,QString> label_and_value)
{
	setInfoLine(this->labelKey_info1, this->label_info1, label_and_value);
}
void TrafficProgressDialog::setInfoLine2(QPair<QString,QString> label_and_value)
{
	setInfoLine(this->labelKey_info2, this->label_info2, label_and_value);
}
void TrafficProgressDialog::setInfoLine3(QPair<QString,QString> label_and_value)
{
	setInfoLine(this->labelKey_info3, this->label_info3, label_and_value);
}
void TrafficProgressDialog::setInfoLine4(QPair<QString,QString> label_and_value)
{
	setInfoLine(this->labelKey_info4, this->label_info4, label_and_value);
}
void TrafficProgressDialog::setInfoLines(StringPairList label_and_values)
{
	QPair<QString,QString> empty_info;
	if (label_and_values.size() > 0) {
		setInfoLine1(label_and_values[0]);
	} else {
		setInfoLine1(empty_info);
	}
	if (label_and_values.size() > 1) {
		setInfoLine2(label_and_values[1]);
	} else {
		setInfoLine1(empty_info);
	}
	if (label_and_values.size() > 2) {
		setInfoLine3(label_and_values[2]);
	} else {
		setInfoLine1(empty_info);
	}
	if (label_and_values.size() > 3) {
		setInfoLine4(label_and_values[3]);
	} else {
		setInfoLine1(empty_info);
	}
}

void TrafficProgressDialog::updateProgress( const QString& /*taskText*/, float percentFinished, const QDateTime& /*timeRemaining*/, StringPairList infos )
{
	if (!Settings::SHOW_PROGRESS) return;

	// qDebug() << "TrafficProgressDialog::updateProgress(" << "," << taskText << "," << percentFinished << "," << timeRemaining << "," << infos << ")";
	// TODO: use the other parameters to fill GUI-elements
	this->progressBar->setValue((int)(percentFinished*100.0f));

	const int msecs_to_wait_for_flush = 500;
	QDateTime currentTime = QDateTime::currentDateTime();
	this->lastInfos = infos;
	if ( this->lastUpdateTextInfo.addMSecs(msecs_to_wait_for_flush) <= currentTime )
	{
		// update, but only after a second, such that following info arriving in the next second are also flushed
		this->lastUpdateTextInfo = currentTime;
		QTimer::singleShot(msecs_to_wait_for_flush, this, SLOT(flushInfoLines()));
	}
}

void TrafficProgressDialog::flushInfoLines()
{
	qDebug() << "TrafficProgressDialog::flushInfoLines() -> lastInfos " << lastInfos;
	this->setInfoLines(lastInfos);
}
