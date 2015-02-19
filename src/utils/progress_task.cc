/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2009-2011 stepping stone GmbH
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
#include <QWaitCondition>
#include <QMutex>
#include <math.h>
#include <algorithm>

#include "utils/progress_task.hh"

const double ProgressTask::DURATION_CALCULATION_TIME_RANGE = 40.0; // only the activities of the last TIME_RANGE seconds are taken into Account
const double ProgressTask::MEAN_OVER_LAST_N_SECONDS = 20.0;


ProgressTask::ProgressTask() :
	isFinished(false), fp_changed(false), parent(0), nSteps(0)
{
}
ProgressTask::ProgressTask(const QString& name, const QDateTime& estDuration, const double nSteps) :
	isFinished(false), fp_changed(true), parent(0)
{
	setName(name);
	setEstimatedDuration(estDuration);
	setNumberOfSteps(nSteps);
}

ProgressTask::ProgressTask(ProgressTask* parent, QString name, const QDateTime& estDuration, const double nSteps) :
	isFinished(false), fp_changed(true), parent(parent)
{
	setName(name);
	setEstimatedDuration(estDuration);
	setNumberOfSteps(nSteps);
	this->setParent(parent);
}

ProgressTask::~ProgressTask() {
	for (int i = 0; i < subTasks.size(); i++) {
		delete subTasks.takeLast();
	}
}

bool ProgressTask::existsSubtask(QString name)
{
	for (int i = 0; i < this->subTasks.size(); i++)
		if (this->subTasks[i]->getName() == name)
			return true;
	return false;
}

bool ProgressTask::existsSubtask(int i)
{
	return (i >= 0 && i < this->subTasks.size());
}

ProgressTask* ProgressTask::getSubtask(const QString& name)
{
	for (int i = 0; i < this->subTasks.size(); i++)
		if (this->subTasks[i]->getName() == name)
			return this->subTasks[i];
	return 0;
}

ProgressTask* ProgressTask::getSubtask(int i)
{
	if (existsSubtask(i))
		return this->subTasks[i];
	return this->appendTask();
}

int ProgressTask::getNumberOfSubtasks()
{
	return this->subTasks.size();
}

ProgressTask* ProgressTask::appendTask(ProgressTask* pt)
{
	this->subTasks.append(pt);
	pt->setParent(this);
	this->setFixpointsChanged(true);
	return pt;
}

ProgressTask* ProgressTask::appendTask()
{
	ProgressTask* pt = new ProgressTask();
	pt->setParent(this);
	this->subTasks.append(pt);
	this->setFixpointsChanged(true);
	return pt;
}

ProgressTask* ProgressTask::appendTask(const QString& name, const QDateTime& estDuration, const double nSteps)
{
	ProgressTask* pt = new ProgressTask(name, estDuration, nSteps);
	pt->setParent(this);
	this->subTasks.append(pt);
	return pt;
}

ProgressTask* ProgressTask::getCurrentTask()
{
	if (this->subTasks.size() > 0)
	{
		int i = 0, i_contains_fp = 0, i_last_terminated;
		// find last terminated task
		for (i = 0; i < this->subTasks.size() && this->subTasks[i]->isTerminated(); i++);
		i_last_terminated = i-1;
		i_contains_fp = std::min<int>(this->subTasks.size()-1,i_last_terminated+1);
		// find (beginning at task last_terminated) the last task, which contains already fixpoints.

		for (i = i_last_terminated+1; i < this->subTasks.size(); i++) {
			if (this->subTasks[i]->fp_steps.size() > 0) {
				i_contains_fp = i;
			}
		}
		return this->subTasks[i_contains_fp]->getCurrentTask();
	}
	else
	{
		return this;
	}
}

void ProgressTask::addFixpointNow(double stepN)
{
	addFixpoint(QDateTime::currentDateTime(), stepN, false);
}

void ProgressTask::addFixpoint(QDateTime time, double stepN, bool force)
{
	if (this->fp_steps.size() > 0 && stepN < this->fp_steps.last()) {
		//qDebug() << "ProgressTask::addFixpoint: WARNING: provided step smaller than last one -> ignored";
	} else {
		if (force || this->fp_times.size()==0 || (DateTimeUtils::getSeconds(time) - DateTimeUtils::getSeconds(this->fp_times.last()) > 0.2) ) {
			this->fp_steps.append(std::max<double>(0.0,std::min<double>(this->nSteps,stepN)));
			this->fp_times.append(time);
			this->setFixpointsChanged(true);
		}
	}
}

void ProgressTask::addFixpointNowForce(double stepN)
{
	addFixpoint(QDateTime::currentDateTime(), stepN, true);
}

QDateTime ProgressTask::getStartTime()
{
	if (this->fp_times.size() > 0) {
		return this->fp_times[0];
	} else {
		return QDateTime();
	}
}

QDateTime ProgressTask::getRunningTime() {
	if (this->fp_times.size() > 0) {
		return(DateTimeUtils::getDateTimeFromSecs( DateTimeUtils::getSeconds(fp_times.last()) - DateTimeUtils::getSeconds(this->getStartTime()) ));
	} else {
		return(QDateTime::currentDateTime());
	}
}

/**
 * returns either the estimated total duaration, if there are fixpoints available or the provided initial guess
 */
QDateTime ProgressTask::getTotalDurationBestGuess()
{
	if (fp_steps.size() >= 2) {
		return DateTimeUtils::getDateTimeFromSecs( std::max<double>(0.0,DateTimeUtils::getSeconds(this->getEstimatedTotalDuration())) );
	} else {
		return this->getProposedEstimatedDuration();
	}
}


void ProgressTask::setFixpointsChanged(bool fp_changed)
{
	this->fp_changed = fp_changed;
	if (fp_changed && this->parent != 0) {
		this->parent->setFixpointsChanged(true);
	}
}

bool ProgressTask::haveFixpointsChanged()
{
	return fp_changed;
}

void ProgressTask::collectFixpointsFromSubtasks()
{
	if (this->haveFixpointsChanged()) {
		if (this->subTasks.size() > 0) {
			// in this case the own fixpoints are filled by a normalized set of the fixpoints of the subTasks
			ProgressTask fp_new;
			double totDuration = 0;
			this->fp_times.clear();
			this->fp_steps.clear();
			for (int i = 0; i < subTasks.size(); i++) {
				ProgressTask* pt = subTasks[i];
				pt->collectFixpointsFromSubtasks();
				double q = DateTimeUtils::getSeconds(pt->getProposedEstimatedDuration()) / pt->getNumberOfSteps();
				for (int j = 0; j < pt->fp_times.size(); j++) {
					this->fp_steps.append(pt->fp_steps[j] * q + totDuration );
					this->fp_times.append(pt->fp_times[j]);
				}
				// totDuration += DateTimeUtils::getSeconds( pt->getTotalDurationBestGuess() ); // why is this a bad idea?
				totDuration += DateTimeUtils::getSeconds( pt->getProposedEstimatedDuration() );
			}
			this->setNumberOfSteps(totDuration); // for normalization, time is the linear measure for the steps
			this->setEstimatedDuration(DateTimeUtils::getDateTimeFromSecs(totDuration));
		}
		this->setFixpointsChanged(false);
	}
}

QDateTime ProgressTask::getEstimatedTimeLeft()
{
	// gather Fixpoints from the subtasks
	this->collectFixpointsFromSubtasks();
	if (this->fp_steps.size() < 2) {
		if (fp_times.size() > 0) {
			// if starting time is provided, count backward based on estimated duration
			return(DateTimeUtils::getDateTimeFromSecs( std::max<double>(0.0,DateTimeUtils::getSeconds(this->getProposedEstimatedDuration()) + DateTimeUtils::getSeconds(fp_times.last()) - DateTimeUtils::getSeconds(QDateTime::currentDateTime())) ));
		} else {
			// if no starting time is provided, return estimated duration
			return this->getProposedEstimatedDuration();
		}
	} else {
		double t_mean = 0.0;
		double n_mean = 0.0;
		double t_last_secs = DateTimeUtils::getSeconds(fp_times.last());
		int i_from = fp_steps.size()-2; // earliest fixpoint to take into account
		while (i_from > 0 && t_last_secs - DateTimeUtils::getSeconds(fp_times[i_from])<=DURATION_CALCULATION_TIME_RANGE) i_from--;
		int n = fp_steps.size()-i_from;
		QVector<double> times(n);
		QVector<double> steps(n);
		for (int i = 0; i < n; i++) {
			int j = i + i_from;
			times[i] = DateTimeUtils::getSeconds(fp_times[j]);
			t_mean += times[i];
			steps[i] = fp_steps[j];
			n_mean += steps[i];
		}
		t_mean /= n; n_mean /= n;
		double sum_dxdy = 0, sum_dx2 = 0;
		for (int i = 0; i < n; i++) {
			sum_dxdy += (times[i]-t_mean)*(steps[i]-n_mean);
			sum_dx2  += pow((steps[i]-n_mean),2.0);
		}
		double remaining_secs_from_last_fp = (this->getNumberOfSteps() - fp_steps.last()) * sum_dxdy / sum_dx2;
		QDateTime guess = DateTimeUtils::getDateTimeFromSecs( remaining_secs_from_last_fp + DateTimeUtils::getSeconds(fp_times.last()) - DateTimeUtils::getSeconds(QDateTime::currentDateTime()) );

		// if guess is unrealistically large (>1 month), guess is not accepted as reliable value
		if (DateTimeUtils::getSeconds(guess)<2592000.0) {
			// the estimation is fed into a list, then a mean of these guesses (with respect to the time passed) is returned
			this->lastDurationGuesses.prepend(QPair<QDateTime,QDateTime>(QDateTime::currentDateTime(), guess));
		}
		double retSecs = 0.0, curSecs = DateTimeUtils::getSeconds(QDateTime::currentDateTime());
		int i = -1;
		while (++i < this->lastDurationGuesses.size() && (curSecs-DateTimeUtils::getSeconds(this->lastDurationGuesses[i].first) < this->MEAN_OVER_LAST_N_SECONDS)) {
			retSecs += std::max<double>(0.0,DateTimeUtils::getSeconds(this->lastDurationGuesses[i].first) + DateTimeUtils::getSeconds(this->lastDurationGuesses[i].second) - curSecs);
		}
		this->lastDurationGuesses = this->lastDurationGuesses.mid(0,i);
		if (i > 0) {
			return( DateTimeUtils::getDateTimeFromSecs(retSecs/i) );
		} else {
			return( DateTimeUtils::getDateTimeFromSecs(0) );
		}
	}
}

QDateTime ProgressTask::getEstimatedTotalDuration()
{
	return DateTimeUtils::getDateTimeFromSecs( DateTimeUtils::getSeconds(getEstimatedTimeLeft()) + DateTimeUtils::getSeconds(getStartTime()) - DateTimeUtils::getSeconds(QDateTime::currentDateTime()) );
}

QString ProgressTask::getEstimatedTimeLeftString()
{
	return getEstimatedTimeLeft().toString("h:mm:ss");
}

float ProgressTask::getFinishedRatio()
{
	// return finished ratio based on (only!) the provided duration-estimations and the task-steps done
	this->collectFixpointsFromSubtasks();
	return (float)(this->fp_steps.last() / this->getNumberOfSteps());
}

void ProgressTask::testProgressTaskClass()
{
	qDebug() << "ProgressTask Test";

	qDebug() << "DateTime-Test";
	qDebug() << DateTimeUtils::getSeconds(DateTimeUtils::getDateTimeFromSecs(100));

	ProgressTask pt("Backup", DateTimeUtils::getDateTimeFromSecs(60), 50);
	pt.appendTask("dry-run", DateTimeUtils::getDateTimeFromSecs(40), 30);
	pt.appendTask("file-upload", DateTimeUtils::getDateTimeFromSecs(100), 250);
	pt.appendTask("send meta-information", DateTimeUtils::getDateTimeFromSecs(20), 2);

	for (int task_nr = 0; task_nr < pt.getNumberOfSubtasks(); task_nr++) {
		ProgressTask* subPt = pt.getSubtask(task_nr);
		qDebug() << "TASK" << subPt->getFullName();
		for (int i = 0; i <= subPt->getNumberOfSteps(); i++) {
			if (i!=0) {
				QWaitCondition waitCondition; QMutex mutex; mutex.lock();
				waitCondition.wait( &mutex, (long)(10 + DateTimeUtils::getSeconds(subPt->getProposedEstimatedDuration())/subPt->getNumberOfSteps()*(task_nr==1?2:1)*(rand()%1000)) );
			}
			subPt->addFixpointNow(i);
			qDebug() << i << "of" << subPt->getNumberOfSteps() << pt.getEstimatedTimeLeftString() << " left" << subPt->getRunningTime().toString("h:mm:ss.zzz");
		}
		subPt->setTerminated();
	}
}
