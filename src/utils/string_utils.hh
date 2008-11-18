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

#ifndef STRING_UTILS_HH_
#define STRING_UTILS_HH_

#include <QString>

class StringUtils
{
public:
	static QString encaps(const QString& aStr, const QString& beforeTok = "", const QString& afterTok = "");
	static QString concatSep(const QString& aStr, const QString& bStr = "", const QString& tok = ",");
};

#endif /*STRING_UTILS_HH_*/

inline QString StringUtils::encaps(const QString& aStr, const QString& beforeTok, const QString& afterTok)
{
	if (aStr == "")
		return aStr;
	return beforeTok + aStr + afterTok;
}

inline QString StringUtils::concatSep(const QString& aStr, const QString& bStr, const QString& tok)
{
	if (aStr == "")
		return bStr;
	if (bStr == "")
		return aStr;
	return aStr + tok + bStr;
}
