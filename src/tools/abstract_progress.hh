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

#ifndef PROGRESS_HH
#define PROGRESS_HH

#include <QObject>

/**
 * The AbstractProgress class provides methods for emitting message signals
 * @author Bruno Santschi, santschi@puzzle.ch
 * @version $Author: bsantschi $ $Date: 2008/04/25 05:42:16 $ $Revision: 1.5 $
 */
class AbstractProgress : public QObject
{
	Q_OBJECT

public:

	/**
	 * Destroys the AbstractProgress
	 */
	virtual ~AbstractProgress();

signals:
	void infoSignal( const QString& text );
	void errorSignal( const QString& text );
};

inline AbstractProgress::~AbstractProgress()
{
	
}

#endif
