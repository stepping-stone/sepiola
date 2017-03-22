/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2017 stepping stone GmbH
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

#ifndef DATETIME_UTILS_HH_
#define DATETIME_UTILS_HH_

#include <QString>
#include <math.h>

class DateTimeUtils
{
public:
	static double getSeconds(const QDateTime t);
	static QDateTime getDateTimeFromSecs(double secs);
};

inline double DateTimeUtils::getSeconds(const QDateTime t)
{
	QDateTime t_ = t;
	t_.setTimeSpec(Qt::UTC);
	return t_.toTime_t() + t_.time().msec()/1000.0;
}
inline QDateTime DateTimeUtils::getDateTimeFromSecs(double secs)
{
	QDateTime t;
	t.setTimeSpec(Qt::UTC);
	t.setTime_t((uint)secs);
	t = t.addMSecs((int)((secs-floor(secs))*1000.0));
	return t;
}
#endif /*DATETIME_UTILS_HH_*/

