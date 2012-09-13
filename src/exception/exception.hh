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

#ifndef EXCEPTION_HH
#define EXCEPTION_HH

#include <stdexcept>

#include <QByteArray>
#include <QString>

/**
 * The Exception class is the base class for all exceptions thrown by this application
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class Exception : public std::exception
{
public:

	/**
	 * Constructs an Exception with the given message
	 * @param message the exception's message
	 */
    Exception( const QString& message );

	/**
	 * Destroys the Exception
	 */
    virtual ~Exception() throw();
    
    /**
     * Returns the message of the exception
     * @return the message
     */
	virtual const char* what() const throw();
	
private:
	QByteArray message;
};

inline Exception::Exception( const QString& message )
{
	this->message = message.toLocal8Bit();
}

inline Exception::~Exception() throw()
{
}

inline const char* Exception::what() const throw()
{
	return message;
}

#endif
