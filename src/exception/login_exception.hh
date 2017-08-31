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

#ifndef LOGIN_EXCEPTION_HH
#define LOGIN_EXCEPTION_HH

#include "exception/exception.hh"

/**
 * The LoginException class is used to indicate a wrong password
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class LoginException : public Exception
{
public:
	/**
	 * Constructs a LoginException with the given message
	 * @param message the exception's message
	 */
	LoginException( const QString& message );
};

inline LoginException::LoginException( const QString& message ) : Exception( message ) 
{
}

#endif
