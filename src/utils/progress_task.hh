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

#ifndef PROGRESS_TASK_HH
#define PROGRESS_TASK_HH

#include <QString>
#include <QDateTime>

#include "test/test_manager.hh"
#include "utils/datetime_utils.hh"


/**
 * ProgressTask class is a helper to display progress based on multiple tasks
 * @author Dominic Sydler, sydler@puzzle.ch
 */
class ProgressTask
{
public:
	ProgressTask();
	ProgressTask(const QString& name, const QDateTime& estDuration, const double nSteps);
	ProgressTask(ProgressTask* parent, QString name, const QDateTime& estDuration, const double nSteps);
	~ProgressTask();
	
	inline QString getName() { return name; };
	inline char* getNameCharArray() { return getName().toUtf8().data(); }
	inline QString getFullName() { qDebug() << this << parent << name; if (parent==0) { return name; } else { return (parent->getFullName()+ " - " + name); } };
	inline void setName(const QString& name) { this->name = name; };
	inline double getNumberOfSteps() { return nSteps; }
	inline void setNumberOfSteps(double nSteps) { this->nSteps = nSteps; this->setFixpointsChanged(true); };
	inline bool isTerminated() { return isFinished; };
	inline void setTerminated(bool isFinished = true) { this->isFinished = isFinished; this->setFixpointsChanged(true); };
	inline QDateTime getProposedEstimatedDuration() { return this->estDuration; };
	inline void setEstimatedDuration(QDateTime duration) { this->estDuration = duration; this->setFixpointsChanged(true); };
	inline void setParent(ProgressTask * parent) { this->parent = parent; this->setFixpointsChanged(true); }
	inline ProgressTask * getParent() { return parent; }
	inline ProgressTask * getRootTask() { if (parent != 0) { return parent->getRootTask(); } else { return this; } }

	void setParentTask(ProgressTask* parent);
	bool existsSubtask(QString name);
	bool existsSubtask(int i);
	ProgressTask* getSubtask(const QString& name);
	ProgressTask* getSubtask(int i);
	int getNumberOfSubtasks();
	ProgressTask* appendTask(ProgressTask* pt);
	ProgressTask* appendTask();
	ProgressTask* appendTask(const QString& name, const QDateTime& estDuration, const double nSteps);
	ProgressTask* getCurrentTask();
	
	void addFixpointNow(double stepN);
	void addFixpointNowForce(double stepN);
	void addFixpoint(QDateTime time, double stepN, bool force=true);
	bool haveFixpointsChanged(); // used by collectFixpointsFromSubtasks to avoid unnecessary runs
	void setFixpointsChanged(bool fp_changed=true); // used by collectFixpointsFromSubtasks to avoid unnecessary runs
	void collectFixpointsFromSubtasks();	
	
	QDateTime getStartTime();
	QDateTime getRunningTime();
	QDateTime getEstimatedTotalDuration();
	QDateTime getTotalDurationBestGuess();
	QDateTime getEstimatedTimeLeft();
	QString getEstimatedTimeLeftString();
	float getFinishedRatio();
	
	inline void debugIsCorrectCurrentTask(const QString& expectedName) { QString curName = this->getCurrentTask()->getName(); if (curName.compare(expectedName)!=0) { qDebug() << "current task is wrong! current task:" << curName << "expected task" << expectedName << "-> solution: terminate finished tasks!"; } else { qDebug() << "current task is:" << curName; } };

	static void testProgressTaskClass();

private:
	static const double DURATION_CALCULATION_TIME_RANGE = 40.0; // only the activities of the last TIME_RANGE seconds are taken into Account
	static const double MEAN_OVER_LAST_N_SECONDS = 20.0;
	bool isFinished;
	bool fp_changed;
	QList<ProgressTask*> subTasks;
	QString name;
	ProgressTask* parent;
protected:	
	double nSteps;
	QDateTime estDuration; // estimated duration passed at creation
	QList<QDateTime> fp_times;
	QList<double> fp_steps;
	QList< QPair<QDateTime,QDateTime> > lastDurationGuesses;
};

namespace
{
	// run test by calling sepiola with parameter: -test testProgressTaskClass
	int dummy = TestManager::registerTest( "testProgressTaskClass", ProgressTask::testProgressTaskClass );
}

#endif // PROGRESS_TASK_HH
