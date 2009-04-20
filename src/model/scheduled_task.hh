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

#ifndef SCHEDULEDTASK_H_
#define SCHEDULEDTASK_H_

#include <QList>
#include <QStringList>
#include <QDebug>
#include <QMetaType>

#include "tools/abstract_rsync.hh"
#include "exception/abort_exception.hh"



class ScheduledTask
{
	public:
		enum ScheduleTypeEnum
		{ AFTER_BOOT, AT_WEEKDAYS_AND_TIME, NEVER};
		enum WeekdaysEnum
		{ MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY};

		ScheduledTask();
		ScheduledTask(QSet<WeekdaysEnum> weekdays, QTime timeToRun);
		ScheduledTask(int minutesAfterStartup);

		~ScheduledTask();
		ScheduleTypeEnum getType() const;
		void setType(ScheduleTypeEnum type);
		QSet<WeekdaysEnum> getWeekdays() const;
		QVector<bool> getWeekdaysArray() const;
		void setWeekdays(QSet<WeekdaysEnum> weekdays);
		void clearWeekdays();
		void addWeekday(WeekdaysEnum newWeekday);
		QTime getTimeToRun() const;
		void setTimeToRun(QTime timeToRun);
		int getMinutesAfterStartup() const;
		void setMinutesAfterStartup(int minutesAfterStartup);
		QString toString() const;
		bool equals(const ScheduledTask& scheduledTask) const;
		
		static QMap<WeekdaysEnum, QString> weekdayNames;

	private:
		ScheduleTypeEnum type;
		QSet<WeekdaysEnum> weekdays;
		QTime timeToRun;
		int minutesAfterStartup;
};
Q_DECLARE_METATYPE(ScheduledTask);

#endif /*SCHEDULEDTASK_H_*/
