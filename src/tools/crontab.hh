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

#ifndef CRONTAB_HH
#define CRONTAB_HH

#include <QString>
#include <QTime>

#include "tools/abstract_scheduler.hh"
#include "tools/process.hh"

/**
 * The Crontab class provides methods for using the crontab scheduler
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Crontab : public AbstractScheduler, public Process
{
public:

	/**
	 * Constructs a Crontab
	 */
	Crontab();
	
	/**
	 * Destroys the Crontab
	 */
	virtual ~Crontab();
	
	/**
	 * @see AbstractScheduler::isSchedulingOnStartSupported()
	 */
	virtual bool isSchedulingOnStartSupported();
	
	/**
	 * @see AbstractScheduler::scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] )
	 */
	void scheduleTask( const QString& execName, const QString& cliArgument, const QTime& time, const bool days[] );
	
	/**
	 * This feature is not implemented for crontab
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
	static void testScheduleTask();
	
	/**
	 * Tests the isVixieCron method
	 */
	static void testIsVixieCron();
	
	bool taskExists( const QString& execName,  const QString& cliArgument );
	
private:
	bool schedule( const QString& execName, const QString& cliArgument, const QString& timeSpecification );
	QStringList readAllCrontabEntries();
	bool writeCrontabEntries_Helper( QStringList& crontabEntries, QString& return_error );
	bool writeAllCrontabEntries( QStringList& crontabEntries );
	QStringList removeExistingBackupJob( const QStringList& existingEntries, const QString& execName,  const QString& cliArgument );
	QStringList updateExistingBackupJob( const QStringList& existingEntries, const QString& execName,  const QString& cliArgument );
	QStringList getExistingEntries();
	static bool isVixieCron();
	
	static const QString CRONTAB_NAME;
	static const QString EXPORT_DISPLAY_ARGUMENT;
};

#endif
