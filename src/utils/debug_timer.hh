/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
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

#ifndef DEBUG_TIMER_HH_
#define DEBUG_TIMER_HH_

#include <QTime>

/*
 * example: 
 *   DebugTimer::setMarker(); qDebug() <<  "timer marker set at <code position info>";
 *   qDebug() << "<code position info> " << QString::number(DebugTimer::msecSinceLast(true))+"msecs since last time-marker";    
 */
class DebugTimer
{
private:
	DebugTimer();
public:
	static void setMarker();
	static int msecSinceLast(bool doResetMarker = false);
	static QTime tmpSetter(bool set = false);
};
#endif /*DEBUG_TIMER_HH_*/

inline QTime DebugTimer::tmpSetter(bool set)
{
	static QTime t0;
	if (set) t0 = QTime::currentTime();
	return t0;
}

inline void DebugTimer::setMarker()
{
	tmpSetter(true);
}

inline int DebugTimer::msecSinceLast(bool doResetMarker)
{
	int dt = tmpSetter(false).msecsTo(QTime::currentTime());
	if (doResetMarker) setMarker();
	return dt;
}
