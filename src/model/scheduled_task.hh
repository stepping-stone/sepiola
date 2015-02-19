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

#ifndef SCHEDULEDTASK_H_
#define SCHEDULEDTASK_H_

#include <QVector>
#include <QSet>
#include <QTime>
#include <QDataStream>
#include <QMetaType>

namespace ScheduleRule {
enum ScheduleType
{ AFTER_BOOT, AT_WEEKDAYS_AND_TIME, NEVER};
enum Weekdays
{ MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY};
}

class ScheduledTask
{
	public:

		ScheduledTask();
		ScheduledTask(const ScheduledTask& task);
		ScheduledTask(QSet<ScheduleRule::Weekdays> w, QTime time);
		ScheduledTask(int minutes);

		~ScheduledTask();
		ScheduleRule::ScheduleType getType() const;
		void setType(ScheduleRule::ScheduleType type);
		QSet<ScheduleRule::Weekdays> getWeekdays() const;
		QVector<bool> getWeekdaysArray() const;
		void setWeekdays(QSet<ScheduleRule::Weekdays> weekdays);
		void setWeekdays(QSet<int> weekdaysInt);
		void clearWeekdays();
		void addWeekday(ScheduleRule::Weekdays newWeekday);
		QTime getTimeToRun() const;
		void setTimeToRun(QTime timeToRun);
		int getMinutesAfterStartup() const;
		void setMinutesAfterStartup(int minutesAfterStartup);
		QString toString() const;
		bool equals(const ScheduledTask& scheduledTask) const;
		
	private:
		ScheduleRule::ScheduleType type;
		QSet<ScheduleRule::Weekdays> weekdays;
		QTime timeToRun;
		int minutesAfterStartup;
};
QDataStream &operator<<(QDataStream &out, const ScheduledTask& sched);
QDataStream &operator>>(QDataStream &in, ScheduledTask& sched);

Q_DECLARE_METATYPE(ScheduledTask)
Q_DECLARE_METATYPE(ScheduleRule::ScheduleType)
Q_DECLARE_METATYPE(ScheduleRule::Weekdays)

#endif /*SCHEDULEDTASK_H_*/
