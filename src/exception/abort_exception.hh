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

#ifndef ABORT_EXCEPTION_HH
#define ABORT_EXCEPTION_HH

#include "exception/exception.hh"

/**
 * The AbortException class is used to indicate that the user wants to abort the process
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class AbortException : public Exception
{
public:
    /**
     * Constructs a AbortException with the given message
     * @param message the exception's message
     */
    AbortException(const QString &message);
};

inline AbortException::AbortException(const QString &message)
    : Exception(message)
{}

#endif
