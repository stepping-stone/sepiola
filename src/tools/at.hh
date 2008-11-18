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

#ifndef AT_HH
#define AT_HH

#include <QLocale>
#include <QString>
#include <QTime>

#include "tools/abstract_scheduler.hh"
#include "tools/process.hh"

/**
 * The At class provides methods for using the "at" scheduler on windows.
 * The german version of Microsoft Windows seems to be the only Windows version, where
 * the at binary has been localized. The getTranslationFor* methods could be
 * enlarged if there is another localized binary.
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: dtschan $ $Date: 2008/06/30 19:18:21 $ $Revision: 1.13 $
 */
class At : public AbstractScheduler, public Process
{
public:

	/**
	 * Constructs an At
	 */
	At();

	/**
	 * Destroys the At
	 */
	virtual ~At();

	/**
	 * @see AbstractScheduler::isSchedulingOnStartSupported()
	 */
	virtual bool isSchedulingOnStartSupported();

	/**
	 * @see AbstractScheduler::scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] )
	 */
	void scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] );

	/**
	 * @see AbstractScheduler::scheduleTask( const QString& execName, const QString& cliArgument, const int& minutesToDelay )
	 */
	void scheduleTask( const QString& execName, const QString& cliArgument, const int& minutesToDelay );

	/**
	 * @see AbstractScheduler::updateExistingTask( const QString& execName, const QString& cliArgument )
	 */
	virtual bool updateExistingTask( const QString& execName, const QString& cliArgument );

	/**
	 * @see AbstractScheduler::deleteExistingTask( const QString& cliArgument )
	 */
	virtual void deleteExistingTask( const QString& execName, const QString& cliArgument );

	/**
	 * Tests the scheduleTask method
	 */
	static void testAtScheduleTask();

private:
	static const QString AT_NAME;

	static QStringList getWeekdayNames( const QLocale& locale );

	QLocale getAtBinaryLocale();
};

inline void At::scheduleTask( const QString& execName, const QString& cliArgument, const int& minutesToDelay )
{
	qWarning() << "Scheduling on start is not supported, test this with AbstractScheduler::isSchedulingOnStartSupported() before calling this method";
}

#endif
