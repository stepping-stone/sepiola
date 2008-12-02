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

#ifndef CLI_MANAGER_HH
#define CLI_MANAGER_HH

#include <QObject>

/**
 * The CliManager class is used to start the application without a graphical user interface.
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class CliManager : public QObject
{
	Q_OBJECT
	
public:
	/**
	 * Argument name for starting the application for the scheduled job
	 */
	static const QString SCHEDULE_ARGUMENT;
	
	/**
	 * Tests if the NOOP_ARGUMENT is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if the NOOP_ARGUMENT is in the argument list	
	 */
	static bool isNoopApplication( int argc, char* argv[] );

	/**
	 * Tests if the HELP_ARGUMENT is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if the HELP_ARGUMENT is in the argument list	
	 */
	static bool isHelpApplication( int argc, char* argv[] );
	
	/**
	 * Prints the usage of this application
	 * @param argc number of arguments
	 * @param argv argument values
	 */
	static void printUsage( int argc, char* argv[] );

	/**
	 * Tests if the CLI_ARGUMENT is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if the CLI_ARGUMENT is in the argument list	
	 */
	static bool isCliApplication( int argc, char* argv[] );
	
	/**
	 * Tests if the SCHEDULE_ARGUMENT is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if the CLI_ARGUMENT is in the argument list	
	 */
	static bool isScheduleApplication( int argc, char* argv[] );
	
	/**
	 * Tests if the SCHEDULE_ARGUMENT is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if the CLI_ARGUMENT is in the argument list	
	 */
	static bool isUpdateOnlyApplication( int argc, char* argv[] );
	
	/**
	 * Runs the scheduled job
	 */
	static void runSchedule();

	/**
	 * Constructs a CliManager
	 */
	CliManager();
	
	/**
	 * Destroys the CliManager
	 */
	virtual ~CliManager();

	
	/**
	 * Runs the application without a graphical user interface
	 * @param argc number of arguments
	 * @param argv argument values
	 */
	void runCli( int argc, char* argv[] );

private slots:
	void printInfoMessage( const QString& message );	
	void printErrorMessage( const QString& message );	
	void askForPassword();
	
private:
	static const QString NOOP_ARGUMENT;
	static const QString HELP_ARGUMENT;
	static const QString CLI_ARGUMENT;
	static const QString UPDATE_ONLY_ARGUMENT;
	static const QString INCLUDE_PATTERN_FILE_ARGUMENT;
	static const QString EXCLUDE_PATTERN_FILE_ARGUMENT;
	static const QString BACKUP_LIST_FILE_ARGUMENT;
	static const QString DELETE_ARGUMENT;

	static bool hasArgument( int argc, char* argv[], const QString& argumentName );
	static bool assertArguments( int argc, char* argv[] );
	static bool hasArgumentValue( int argc, char* argv[], const QString& argumentName  );
	static QString getArgumentsValue( int argc, char* argv[], const QString& argumentName );
	static QString readPassword();
};

#endif
