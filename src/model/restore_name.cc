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

#include <QStringList>
#include <QLocale>

#include "model/restore_name.hh"

RestoreName::RestoreName( const QString& absoluteDirName, const QDate& date  )
{
	this->absoluteDirName = absoluteDirName;
	this->text = QLocale::system().toString(date, QLocale::LongFormat);
	this->date = date;
}

RestoreName::~RestoreName()
{
}

QString RestoreName::getAbsoluteDirName() const
{
	return this->absoluteDirName;	
}

QString RestoreName::getText() const
{
	return this->text;
}

QDate RestoreName::getDate() const
{
	return this->date;
}

bool RestoreName::operator<( const RestoreName& other) const
{
	return other.getDate() < this->getDate();
}

bool RestoreName::operator==( const RestoreName& other) const
{
	return other.getDate() == this->getDate();
}
