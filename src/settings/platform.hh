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

#ifndef PLATFORM_HH
#define PLATFORM_HH

#include <QtCore/QtGlobal>

namespace Platform {

static const char SYSTEM_INDEPENDENT_EOL_CHARACTER[] = "\n";

#ifdef Q_OS_WIN32
static const char EOL_CHARACTER[] = "\r\n";
static const char EXECUTABLE_SUFFIX[] = ".exe";
static const char ROOT_PREFIX[] = "/cygdrive";
#else
static const char EOL_CHARACTER[] = "\n";
static const char EXECUTABLE_SUFFIX[] = "";
static const char ROOT_PREFIX[] = "/";
#endif

#if defined Q_OS_WIN32
static const QString OS_IDENTIFIER = "Windows";
#elif defined Q_OS_MAC
static const QString OS_IDENTIFIER = "MacOSX";
#elif defined Q_OS_LINUX
static const QString OS_IDENTIFIER = "Linux";
#else
#error "Operating system not supported"
#endif

} // namespace Platform
#endif // PLATFORM_HH
