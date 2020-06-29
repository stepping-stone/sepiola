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

#ifndef RESTORE_NAME_HH
#define RESTORE_NAME_HH

#include <QString>
#include <QDate>

/**
 * The RestoreName class represents a restore, containing the remote path to the backup and it's date
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class RestoreName
{
public:
	/**
	 * Constructs a RestoreName
	 * @param absoluteDirName the path of the restore
	 * @param date the date of the backup
	 */
	RestoreName( const QString& absoluteDirName, const QDate& date );
	
	/**
	 * Destroys the RestoreName
	 */
	virtual ~RestoreName();
	
	/**
	 * Gets the absolute directory name
	 * @return absolute directory name
	 */
	QString getAbsoluteDirName() const;

	/**
	 * Gets a nice text representing the date of the backup
	 * @return date of the backup as text
	 */
	QString getText() const;
	
	/**
	 * Gets the date of the backup
	 * @return date of the backup
	 */
	QDate getDate() const;
	
	/**
	 * Returns true if the date of this RestoreName is earlier than other; otherwise returns false.
	 * @param other another RestoreName object
	 */
	bool operator<( const RestoreName& other) const;
	bool operator==( const RestoreName& other) const;
		
private:
	QString absoluteDirName;
	QString text;
	QDate date;
};

#endif
