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

#include "utils/string_utils.hh"
#include "settings/settings.hh"

QString StringUtils::fromLocalEnc(QByteArray byteArray)
{
#ifdef Q_OS_WIN32
    return QString::fromUtf8(byteArray);
#elif defined Q_OS_MAC
    return QString::fromUtf8(byteArray).normalized(QString::NormalizationForm_C);
#else
    return QString::fromLocal8Bit(byteArray);
#endif
}
