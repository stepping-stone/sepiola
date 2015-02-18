/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2012 stepping stone GmbH
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
#include <QtAlgorithms>
#include <QTime>

#include "model/scheduled_task.hh"

/**
 * Constructor for "Never"-Scheduler
 */
ScheduledTask::ScheduledTask()
{
	this->setType(ScheduleRule::NEVER);
	this->clearWeekdays();
	this->setTimeToRun(QTime());
	this->setMinutesAfterStartup(0);
}

/**
 * Constructor for Weekdays-Scheduler
 */
ScheduledTask::ScheduledTask(QSet<ScheduleRule::Weekdays> weekdays, QTime timeToRun)
{
	ScheduledTask();
	this->setType(ScheduleRule::AT_WEEKDAYS_AND_TIME);
	this->setWeekdays(weekdays);
	this->setTimeToRun(timeToRun);
	this->setMinutesAfterStartup(0);
}

/**
 * Constructor for AfterBoot-Scheduler
 */
ScheduledTask::ScheduledTask(int minutesAfterStartup)
{
	ScheduledTask();
	this->setType(ScheduleRule::AFTER_BOOT);
	this->setMinutesAfterStartup(minutesAfterStartup);
}

/**
 * Copy-constructor
 */
ScheduledTask::ScheduledTask(const ScheduledTask& task)
{
	this->type = task.getType();
	this->weekdays = task.getWeekdays();
	this->timeToRun = task.getTimeToRun();
	this->minutesAfterStartup = task.getMinutesAfterStartup();
}

ScheduledTask::~ScheduledTask() {}

ScheduleRule::ScheduleType ScheduledTask::getType() const
{
	return this->type;
}

void ScheduledTask::setType(ScheduleRule::ScheduleType type)
{
	this->type = type;
}

QSet<ScheduleRule::Weekdays> ScheduledTask::getWeekdays() const
{
	return this->weekdays;
}

QVector<bool> ScheduledTask::getWeekdaysArray() const
{
	QVector<bool> days;
	for (int i = 0; i < 7; i++) {
		days.append(weekdays.contains((ScheduleRule::Weekdays)i));
	}
	return days;
}

void ScheduledTask::setWeekdays(QSet<ScheduleRule::Weekdays> weekdays)
{
	this->weekdays = weekdays;
}

void ScheduledTask::setWeekdays(QSet<int> weekdaysInt)
{
	QList<int> wdIntList = weekdaysInt.toList();
	weekdays.clear();
	for (int i = 0; i < wdIntList.size(); i++) {
		weekdays.insert((ScheduleRule::Weekdays)wdIntList.at(i));
	}
}

void ScheduledTask::clearWeekdays()
{
	this->weekdays.clear();
}

void ScheduledTask::addWeekday(ScheduleRule::Weekdays newWeekday)
{
	this->weekdays.insert(newWeekday);
}

QTime ScheduledTask::getTimeToRun() const
{
	return this->timeToRun;
}

void ScheduledTask::setTimeToRun(QTime timeToRun)
{
	this->timeToRun = timeToRun;
}

int ScheduledTask::getMinutesAfterStartup() const
{
	return this->minutesAfterStartup;
}

void ScheduledTask::setMinutesAfterStartup(int minutesAfterStartup)
{
	this->minutesAfterStartup = minutesAfterStartup;
}

QString ScheduledTask::toString() const
{
	switch (this->type)
	{
		case ScheduleRule::NEVER:
        {
            return QString(QObject::tr("never"));
        }

		case ScheduleRule::AT_WEEKDAYS_AND_TIME:
		{
			if (weekdays.size() > 0) {
				//QString tok = ", ";
				QList<ScheduleRule::Weekdays> wdList = weekdays.toList();
				QList<QDateTime> nextBackupDatesList;
				const QDateTime now = QDateTime::currentDateTime();
				const int wd_now = now.date().dayOfWeek();
				//qStableSort(wdList.begin(), wdList.end());
				//QString wdStr = "";
				for (int i = 0; i < wdList.size(); i++)
				{
					QDateTime nextBkup = QDateTime::currentDateTime();
					nextBkup.setTime(this->timeToRun);
					int daysDiff = (wdList.at(i)+1-wd_now+7) % 7; // ScheduleRule::Weekdays starts at Monday=0 -> +1
					if (daysDiff == 0 && (nextBkup.time() < now.time())) { daysDiff = daysDiff + 7; }
					nextBkup = nextBkup.addDays( daysDiff );
					nextBackupDatesList.append(nextBkup);
				}
				//wdStr.chop(tok.length());
				qStableSort(nextBackupDatesList.begin(), nextBackupDatesList.end());

				// full info (not desired by stst
				// return QObject::tr("%3 (on %1 at %2)").arg(wdStr, this->timeToRun.toString("hh:mm"), nextBackupDatesList.at(0).toString());
				return QObject::tr("%1").arg(nextBackupDatesList.at(0).toString("dddd,\tdd.MM.yyyy  hh:mm"));
            } else {
    			return QObject::tr("no weekdays selected");
            }
		}

		case ScheduleRule::AFTER_BOOT:
        {
            return QObject::tr("%1 minutes after startup").arg(this->minutesAfterStartup);
        }
	}
	return QString();
}

bool ScheduledTask::equals(const ScheduledTask& scheduledTask) const
{
	if (this->getType() == scheduledTask.getType()) {
		switch (type) {
			case ScheduleRule::NEVER: return(true); break;
			case ScheduleRule::AFTER_BOOT: return(this->getMinutesAfterStartup() == scheduledTask.getMinutesAfterStartup()); break;
			case ScheduleRule::AT_WEEKDAYS_AND_TIME: return(this->getTimeToRun() == scheduledTask.getTimeToRun() && this->getWeekdays() == scheduledTask.getWeekdays()); break;
			default: return false;
		}
		return false;
	} else {
		return false;
	}
}

QDataStream &operator<<(QDataStream &out, const ScheduledTask& sched)
{
	out << sched.getType() << sched.getWeekdays() << sched.getTimeToRun() << sched.getMinutesAfterStartup();
	return(out);
}

QDataStream &operator>>(QDataStream &in, ScheduledTask& sched)
{
	int type;
	QSet<int> weekdays;
	QTime timeToRun;
	int minutesAfterStartup;
	in >> type >> weekdays >> timeToRun >> minutesAfterStartup;
	sched.setType((ScheduleRule::ScheduleType)type);
	sched.setWeekdays(weekdays);
	sched.setTimeToRun(timeToRun);
	sched.setMinutesAfterStartup(minutesAfterStartup);
	return(in);
}
