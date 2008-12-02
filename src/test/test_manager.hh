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

#ifndef TEST_MANAGER_HH
#define TEST_MANAGER_HH

#include <QString>
#include <QMap>

/**
 * The TestManager class is used to test the application while development
 * @author Daniel Tschan, tschan@puzzle.ch
 */
class TestManager
{
public:

	/**
	 * Registers a test method with a given name
	 * @param testName the name of the test
	 * @param (*fun)() reference to the method
	 * @return just a dummy value
	 */
	static int registerTest( const QString& testName, void (*fun)() );
	
	/**
	 * Tests if a test argument is present in the start argument list 
	 * @param argc number of arguments
	 * @param argv argument values
	 * @return true if a test argument is in the argument list	
	 */
	static bool isTestApplication( int argc, char* argv[] );
	
	/**
	 * Runs the given tests
	 * @param argc number of arguments
	 * @param argv argument values
	 */
	static void run( int argc, char* argv[] );

private:
	static QMap< QString, void (*)() >& functionMap();
	static void runTest( const QString& testName);
	static QStringList getTests( int argc, char* argv[] );
};

inline int TestManager::registerTest( const QString& testName, void (*fun)() )
{
	functionMap().insert( testName, fun );
	
	return 0;
}

inline void TestManager::runTest( const QString& testName)
{
	(*functionMap()[ testName ])();
}

inline QMap< QString, void (*)() >& TestManager::functionMap()
{
	static QMap< QString, void (*)() > map;
	
	return map;
}

#endif
