/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2010-2017 stepping stone GmbH
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

#ifndef EXTENDED_QPROCESS_HH
#define EXTENDED_QPROCESS_HH

#include <QProcess>

class ExtendedQProcess : public QProcess
{
public:
	void resetError();
	bool hasError();

private:
	QString noErrorString;
};

inline void ExtendedQProcess::resetError()
{
	// There's no way to reset error, so reset errorString only.
	setErrorString("");
	this->noErrorString = errorString();
}

inline bool ExtendedQProcess::hasError()
{
	return errorString() != this->noErrorString;
}

#endif

