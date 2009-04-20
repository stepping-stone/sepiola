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
#include <QtAlgorithms>
#include <QTime>

#include "model/scheduled_task.hh"


QMap<ScheduledTask::WeekdaysEnum, QString> ScheduledTask::weekdayNames;

/**
 * Constructor for "Never"-Scheduler
 */
ScheduledTask::ScheduledTask()
{
//ScheduledTask::hallo = 1;
	this->weekdayNames.insert(MONDAY, QObject::tr("Monday"));
	this->weekdayNames.insert(TUESDAY, QObject::tr("Tuesday"));
	this->weekdayNames.insert(WEDNESDAY, QObject::tr("Wednesday"));
	this->weekdayNames.insert(THURSDAY, QObject::tr("Thursday"));
	this->weekdayNames.insert(FRIDAY, QObject::tr("Friday"));
	this->weekdayNames.insert(SATURDAY, QObject::tr("Saturday"));
	this->weekdayNames.insert(SUNDAY, QObject::tr("Sunday"));
	this->setType(NEVER);
	this->clearWeekdays();
	this->setTimeToRun(QTime());
	this->setMinutesAfterStartup(-1);
}

/**
 * Constructor for Weekdays-Scheduler
 */
ScheduledTask::ScheduledTask(QSet<WeekdaysEnum> weekdays, QTime timeToRun)
{
	ScheduledTask();
	this->setType(ScheduledTask::AT_WEEKDAYS_AND_TIME);
	this->setWeekdays(weekdays);
	this->setTimeToRun(timeToRun);
}

/**
 * Constructor for AfterBoot-Scheduler
 */
ScheduledTask::ScheduledTask(int minutesAfterStartup)
{
	ScheduledTask();
	this->setType(AFTER_BOOT);
	this->setMinutesAfterStartup(minutesAfterStartup);
}

ScheduledTask::~ScheduledTask() {}

ScheduledTask::ScheduleTypeEnum ScheduledTask::getType()
{
	return this->type;
}

void ScheduledTask::setType(ScheduledTask::ScheduleTypeEnum type)
{
	this->type = type;
}

QSet<ScheduledTask::WeekdaysEnum> ScheduledTask::getWeekdays()
{
	return this->weekdays;
}

void ScheduledTask::setWeekdays(QSet<WeekdaysEnum> weekdays)
{
	this->weekdays = weekdays;
}

void ScheduledTask::clearWeekdays()
{
	this->weekdays.clear();
}

void ScheduledTask::addWeekday(WeekdaysEnum newWeekday)
{
	this->weekdays.insert(newWeekday);
}

QTime ScheduledTask::getTimeToRun()
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

QString ScheduledTask::toString()
{
	switch (this->type)
	{
		case NEVER: { return QString(QObject::tr("never")); break; }
		case AT_WEEKDAYS_AND_TIME:
		{
			QString tok = ", ";
			QList<WeekdaysEnum> wdList = weekdays.toList();
			qStableSort(wdList.begin(), wdList.end());
			QString wdStr = "";
			for (int i = 0; i < wdList.size(); i++)
			{
				wdStr = wdStr + this->weekdayNames.value(wdList.at(i)).left(3) + tok;
			}
			wdStr.chop(tok.length());
			return QObject::tr("on %1 at %2").arg(wdStr, this->timeToRun.toString("hh:mm"));
			break;
		}
		case AFTER_BOOT:
		{
			return QObject::tr("%1 minutes after starup").arg(this->minutesAfterStartup);
			break;
		}
		default:
			return "";
	}
	return "";
}

bool ScheduledTask::equals(const ScheduledTask& scheduledTask)
{
	if (this->type == scheduledTask.getType()) {
		switch (type) {
			case ScheduledTask::NEVER: return(true); break;
			case ScheduledTask::AFTER_BOOT: return(this->minutesAfterStartup == scheduledTask.getMinutesAfterStartup()); break;
			case ScheduledTask::AT_WEEKDAYS_AND_TIME: return(this->timeToRun == scheduledTask.getTimeToRun() && this->weekdays == scheduledTask.getWeekdays()); break;
			default: return false;
		}
		return false;
	} else {
		return false;
	}
}
