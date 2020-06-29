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

#ifndef SCHTASKS_HH
#define SCHTASKS_HH

#include <QLocale>
#include <QString>
#include <QTime>

#include "tools/abstract_scheduler.hh"
#include "tools/process.hh"

/**
 * The Schtasks class provides methods for using the "schtasks" scheduler on windows.
 * The german version of Microsoft Windows seems to be the only Windows version, where
 * the schtasks binary has been localized. The getTranslationFor* methods could be
 * enlarged if there is another localized binary.
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Schtasks : public AbstractScheduler, public Process
{
public:

	/**
	 * Constructs an Schtasks
	 */
	Schtasks();
	
	/**
	 * Destroys the Schtasks
	 */
	virtual ~Schtasks();

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
	 * @return bool-value, if correct password has been stored in password-argument
	 * on return-value true, password is modified to the (correct!) entered password.
	 * incorrect passwords result in requerying, cancel results to return-value false 
	 */
	bool getWindowsPassword(const QString& username, QString& password, const QString& msg = "");
	
	bool taskExists(QString taskName = "");

	bool updateExistingTask(const QString& taskName);
	
	/**
	 * @see AbstractScheduler::updateExistingTask( const QString& execName, const QString& cliArgument )
	 */
	virtual bool updateExistingTask( const QString& execName, const QString& cliArgument );

	void deleteExistingTask(const QString& taskName);
	
	/**
	 * @see AbstractScheduler::deleteExistingTask( const QString& cliArgument )
	 */
	virtual void deleteExistingTask( const QString& execName, const QString& cliArgument );

	/**
	 * Tests whether schtasks is supported on the current operating system
	 * @return flag if schtasks is supported
	 */
	static bool isSchtasksSupported();

	/**
	 * Tests the scheduleTask method
	 */
	static void testSchtasksScheduleTask();
	
private:
	static const QString SCHTASKS_NAME;
	
	bool schedule( const QString& execName, const QString& cliArgument, const QStringList& schtasksArguments, const QLocale& locale );
	QLocale getSchtasksBinaryLocale();
	QString getCodecName();
	QString getTranslationForWrongAccountInformation(const QLocale& locale);
	QString getTranslationForPassword(const QLocale& locale);
	QString getTranslationForWeekly( const QLocale& locale );
	QString getTranslationForWARNING( const QLocale& locale );
	QString getTranslationForERROR( const QLocale& locale );
	QString getTranslationForOnStart( const QLocale& locale );
};

#endif
