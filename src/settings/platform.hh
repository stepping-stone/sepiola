/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2012 stepping stone GmbH
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

#ifndef PLATFORM_HH
#define PLATFORM_HH

#include <QtCore/QtGlobal>

namespace Platform {

#ifdef Q_OS_WIN32
    static const char EOL_CHARACTER[] = "\r\n";
    static const char EXECUTABLE_SUFFIX[] = ".exe";
#else
    static const char EOL_CHARACTER[] = "\n";
    static const char EXECUTABLE_SUFFIX[] = "";
#endif

}
#endif // PLATFORM_HH
