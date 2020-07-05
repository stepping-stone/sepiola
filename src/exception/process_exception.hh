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

#ifndef PROCESS_EXCEPTION_HH
#define PROCESS_EXCEPTION_HH

#include "exception/exception.hh"
/**
 * The ProcessException class is used to indicate errors of a process
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class ProcessException : public Exception
{
public:
    /**
     * Constructs a ProcessException with the given message
     * @param message the exception's message
     */
    ProcessException(const QString &message);
};

inline ProcessException::ProcessException(const QString &message)
    : Exception(message)
{}

#endif
