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

#ifndef RUNTIME_EXCEPTION_HH_
#define RUNTIME_EXCEPTION_HH_

#include "exception/exception.hh"
/**
 * The RuntimeException class is used to indicate runtime errors
 * @author Dominic Sydler, sydler@puzzle.ch
 */
class RuntimeException : public Exception
{
public:
	/**
	 * Constructs a RuntimeException with the given message
	 * @param message the exception's message
	 */
	RuntimeException( const QString& message );
};

inline RuntimeException::RuntimeException( const QString& message ) : Exception( message ) 
{
}

#endif /*RUNTIME_EXCEPTION_HH_*/
